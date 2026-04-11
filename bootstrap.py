#!/usr/bin/env python3
"""
╔══════════════════════════════════════════════════════════════╗
║  ZKAEDI-CC  ·  Self-Hosting C Compiler  ·  Stage 0           ║
║  Bootstrap: Python implementation → x86-64 Linux assembly    ║
║  Recursively Coupled Hamiltonian Compiler Architecture        ║
╚══════════════════════════════════════════════════════════════╝

Bootstrap chain:
  Stage 0 (this file)  : Python  → compiles compiler.c → cc1
  Stage 1 (cc1)        : C       → compiles compiler.c → cc2
  Self-hosting check   : cc1 == cc2  (binary equivalence)

Usage:
  python3 bootstrap.py source.c -o source          # compile + link
  python3 bootstrap.py source.c -S                 # emit assembly only
  python3 bootstrap.py source.c -c                 # emit object file
"""

import sys, os, re, subprocess, tempfile
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import List, Optional, Dict, Tuple, Any

# ─────────────────────────────────────────────────────────────
# §1  TOKEN TYPES
# ─────────────────────────────────────────────────────────────
class TK(Enum):
    # Literals
    INT_LIT = auto(); LONG_LIT = auto(); CHAR_LIT = auto(); STR_LIT = auto()
    # Identifiers / keywords
    IDENT = auto()
    # Keywords
    AUTO=auto(); BREAK=auto(); CASE=auto(); CHAR=auto(); CONST=auto()
    CONTINUE=auto(); DEFAULT=auto(); DO=auto(); DOUBLE=auto(); ELSE=auto()
    ENUM=auto(); EXTERN=auto(); FLOAT=auto(); FOR=auto(); GOTO=auto()
    IF=auto(); INLINE=auto(); INT=auto(); LONG=auto(); REGISTER=auto()
    RESTRICT=auto(); RETURN=auto(); SHORT=auto(); SIGNED=auto(); SIZEOF=auto()
    STATIC=auto(); STRUCT=auto(); SWITCH=auto(); TYPEDEF=auto(); UNION=auto()
    UNSIGNED=auto(); VOID=auto(); VOLATILE=auto(); WHILE=auto()
    # Operators (multi-char)
    ARROW=auto(); INC=auto(); DEC=auto(); LSHIFT=auto(); RSHIFT=auto()
    LEQ=auto(); GEQ=auto(); EQ=auto(); NEQ=auto(); LAND=auto(); LOR=auto()
    ELLIPSIS=auto()
    ADD_ASSIGN=auto(); SUB_ASSIGN=auto(); MUL_ASSIGN=auto(); DIV_ASSIGN=auto()
    MOD_ASSIGN=auto(); AND_ASSIGN=auto(); OR_ASSIGN=auto(); XOR_ASSIGN=auto()
    LSHIFT_ASSIGN=auto(); RSHIFT_ASSIGN=auto()
    # Single-char operators / punctuation  (stored as their char)
    PUNCT = auto()
    # Special
    EOF = auto()

KEYWORDS = {
    'auto':TK.AUTO,'break':TK.BREAK,'case':TK.CASE,'char':TK.CHAR,
    'const':TK.CONST,'continue':TK.CONTINUE,'default':TK.DEFAULT,'do':TK.DO,
    'double':TK.DOUBLE,'else':TK.ELSE,'enum':TK.ENUM,'extern':TK.EXTERN,
    'float':TK.FLOAT,'for':TK.FOR,'goto':TK.GOTO,'if':TK.IF,
    'inline':TK.INLINE,'int':TK.INT,'long':TK.LONG,'register':TK.REGISTER,
    'restrict':TK.RESTRICT,'return':TK.RETURN,'short':TK.SHORT,
    'signed':TK.SIGNED,'sizeof':TK.SIZEOF,'static':TK.STATIC,
    'struct':TK.STRUCT,'switch':TK.SWITCH,'typedef':TK.TYPEDEF,
    'union':TK.UNION,'unsigned':TK.UNSIGNED,'void':TK.VOID,
    'volatile':TK.VOLATILE,'while':TK.WHILE,
}

@dataclass
class Token:
    kind: TK
    val:  Any       # str / int / None
    line: int
    col:  int
    def __repr__(self): return f"Token({self.kind.name},{self.val!r})"

# ─────────────────────────────────────────────────────────────
# §2  LEXER
# ─────────────────────────────────────────────────────────────
class Lexer:
    def __init__(self, src: str, filename="<stdin>"):
        self.src = src
        self.pos = 0
        self.line = 1
        self.col  = 1
        self.filename = filename
        self.tokens: List[Token] = []
        self._tokenize()

    def _cur(self): return self.src[self.pos] if self.pos < len(self.src) else '\0'
    def _peek(self,n=1): p=self.pos+n; return self.src[p] if p<len(self.src) else '\0'

    def _advance(self):
        c = self.src[self.pos]; self.pos += 1
        if c == '\n': self.line += 1; self.col = 1
        else: self.col += 1
        return c

    def _tok(self, kind, val=None):
        self.tokens.append(Token(kind, val, self.line, self.col))

    def _error(self, msg):
        raise SyntaxError(f"{self.filename}:{self.line}:{self.col}: {msg}")

    def _skip_whitespace_and_comments(self):
        while self.pos < len(self.src):
            c = self._cur()
            if c in ' \t\r\n': self._advance()
            elif c == '/' and self._peek() == '/':
                while self.pos < len(self.src) and self._cur() != '\n': self._advance()
            elif c == '/' and self._peek() == '*':
                self._advance(); self._advance()
                while self.pos < len(self.src):
                    if self._cur() == '*' and self._peek() == '/':
                        self._advance(); self._advance(); break
                    self._advance()
            elif c == '#':
                # Handle preprocessor directives (basic)
                self._handle_directive()
            else: break

    def _handle_directive(self):
        # skip the '#' 
        self._advance()
        # read directive name
        while self._cur() in ' \t': self._advance()
        name = ''
        while self._cur().isalpha(): name += self._advance()
        # skip rest of line (we process #include/#define externally)
        while self.pos < len(self.src) and self._cur() != '\n': self._advance()

    def _read_string(self):
        self._advance()  # skip "
        s = ''
        while self.pos < len(self.src) and self._cur() != '"':
            if self._cur() == '\\':
                self._advance()
                esc = self._advance()
                s += {'n':'\n','t':'\t','r':'\r','0':'\0',
                      '\\':'\\','"':'"',"'":"'",'a':'\a','b':'\b'}.get(esc,'\\'+esc)
            else: s += self._advance()
        if self._cur() != '"': self._error("unterminated string")
        self._advance()
        return s

    def _read_char(self):
        self._advance()  # skip '
        if self._cur() == '\\':
            self._advance()
            esc = self._advance()
            v = ord({'n':'\n','t':'\t','r':'\r','0':'\0',
                     '\\':'\\','\'':'\'','a':'\a','b':'\b'}.get(esc,esc))
        else: v = ord(self._advance())
        if self._cur() != "'": self._error("unterminated char literal")
        self._advance()
        return v

    def _tokenize(self):
        while True:
            self._skip_whitespace_and_comments()
            if self.pos >= len(self.src): self._tok(TK.EOF); break
            line, col = self.line, self.col
            c = self._cur()

            # String literal
            if c == '"':
                s = self._read_string()
                self.tokens.append(Token(TK.STR_LIT, s, line, col)); continue

            # Char literal
            if c == "'":
                v = self._read_char()
                self.tokens.append(Token(TK.CHAR_LIT, v, line, col)); continue

            # Number
            if c.isdigit() or (c == '0' and self._peek() in 'xX'):
                self._read_number(line, col); continue

            # Identifier / keyword
            if c.isalpha() or c == '_':
                s = ''
                while self._cur().isalnum() or self._cur() == '_': s += self._advance()
                kind = KEYWORDS.get(s, TK.IDENT)
                # handle long suffix on next token
                self.tokens.append(Token(kind, s, line, col)); continue

            # Multi-char operators
            two = c + self._peek()
            three = two + self._peek(2)

            if three == '...': self._advance();self._advance();self._advance(); self._tok(TK.ELLIPSIS); continue
            if three == '<<=': self._advance();self._advance();self._advance(); self._tok(TK.LSHIFT_ASSIGN); continue
            if three == '>>=': self._advance();self._advance();self._advance(); self._tok(TK.RSHIFT_ASSIGN); continue

            multi = {
                '->':TK.ARROW,'++':TK.INC,'--':TK.DEC,
                '<<':TK.LSHIFT,'>>':TK.RSHIFT,
                '<=':TK.LEQ,'>=':TK.GEQ,'==':TK.EQ,'!=':TK.NEQ,
                '&&':TK.LAND,'||':TK.LOR,
                '+=':TK.ADD_ASSIGN,'-=':TK.SUB_ASSIGN,'*=':TK.MUL_ASSIGN,
                '/=':TK.DIV_ASSIGN,'%=':TK.MOD_ASSIGN,'&=':TK.AND_ASSIGN,
                '|=':TK.OR_ASSIGN,'^=':TK.XOR_ASSIGN,
            }
            if two in multi:
                self._advance(); self._advance()
                self.tokens.append(Token(multi[two], two, line, col)); continue

            # Single char
            self._advance()
            self.tokens.append(Token(TK.PUNCT, c, line, col))

    def _read_number(self, line, col):
        s = ''
        is_long = False
        if self._cur() == '0' and self._peek() in 'xX':
            s += self._advance(); s += self._advance()
            while self._cur() in '0123456789abcdefABCDEF': s += self._advance()
            val = int(s, 16)
        elif self._cur() == '0':
            while self._cur().isdigit(): s += self._advance()
            val = int(s, 8) if len(s)>1 else int(s)
        else:
            while self._cur().isdigit(): s += self._advance()
            val = int(s)
        # suffix
        if self._cur() in 'lL': self._advance(); is_long = True
        if self._cur() in 'uU': self._advance()
        if self._cur() in 'lL': self._advance(); is_long = True
        kind = TK.LONG_LIT if is_long or val > 2**31-1 else TK.INT_LIT
        self.tokens.append(Token(kind, val, line, col))

# ─────────────────────────────────────────────────────────────
# §3  TYPE SYSTEM
# ─────────────────────────────────────────────────────────────
class TypeKind(Enum):
    VOID=auto(); CHAR=auto(); INT=auto(); LONG=auto(); UCHAR=auto()
    UINT=auto(); ULONG=auto(); PTR=auto(); ARRAY=auto(); STRUCT=auto()
    FUNC=auto()

@dataclass
class CType:
    kind: TypeKind
    # PTR -> base; ARRAY -> base + array_size; STRUCT -> struct_tag + members
    base: Optional['CType'] = None
    array_size: int = 0       # 0 = incomplete
    struct_tag: str = ''
    members: Optional[List[Tuple[str,'CType',int]]] = None  # name,type,offset
    struct_size: int = 0
    params: Optional[List['CType']] = None   # for FUNC
    ret: Optional['CType'] = None
    variadic: bool = False
    is_const: bool = False

    def size(self) -> int:
        if self.kind==TypeKind.VOID: return 0
        if self.kind in (TypeKind.CHAR,TypeKind.UCHAR): return 1
        if self.kind in (TypeKind.INT,TypeKind.UINT): return 4
        if self.kind in (TypeKind.LONG,TypeKind.ULONG,TypeKind.PTR): return 8
        if self.kind==TypeKind.ARRAY:
            return self.base.size() * self.array_size
        if self.kind==TypeKind.STRUCT:
            return self.struct_size
        return 8

    def align(self) -> int:
        if self.kind==TypeKind.ARRAY: return self.base.align()
        if self.kind==TypeKind.STRUCT:
            if not self.members: return 1
            return max(m[1].align() for m in self.members)
        return min(self.size(), 8) if self.size()>0 else 1

    def is_integer(self): return self.kind in (TypeKind.CHAR,TypeKind.UCHAR,TypeKind.INT,TypeKind.UINT,TypeKind.LONG,TypeKind.ULONG)
    def is_pointer(self): return self.kind==TypeKind.PTR
    def is_scalar(self): return self.is_integer() or self.is_pointer()
    def is_signed(self): return self.kind in (TypeKind.CHAR,TypeKind.INT,TypeKind.LONG)

    def __repr__(self):
        if self.kind==TypeKind.PTR: return f"{self.base}*"
        if self.kind==TypeKind.ARRAY: return f"{self.base}[{self.array_size}]"
        return self.kind.name.lower()

# Built-in types
T_VOID  = CType(TypeKind.VOID)
T_CHAR  = CType(TypeKind.CHAR)
T_INT   = CType(TypeKind.INT)
T_LONG  = CType(TypeKind.LONG)
T_UCHAR = CType(TypeKind.UCHAR)
T_UINT  = CType(TypeKind.UINT)
T_ULONG = CType(TypeKind.ULONG)
T_CHARP = CType(TypeKind.PTR, base=T_CHAR)
T_VOIDP = CType(TypeKind.PTR, base=T_VOID)

def ptr_to(t: CType) -> CType: return CType(TypeKind.PTR, base=t)
def arr_of(t: CType, n: int) -> CType: return CType(TypeKind.ARRAY, base=t, array_size=n)

# ─────────────────────────────────────────────────────────────
# §4  AST NODES
# ─────────────────────────────────────────────────────────────
@dataclass
class Node:
    pass

# Declarations
@dataclass
class VarDecl(Node):
    name: str; ty: CType; init: Optional['Node']; is_global: bool=False; is_static: bool=False; is_extern: bool=False

@dataclass
class FuncDecl(Node):
    name: str; ret: CType; params: List[Tuple[str,CType]]; body: Optional['Node']
    is_static: bool=False; is_extern: bool=False

@dataclass
class TranslationUnit(Node):
    decls: List[Node]

# Statements
@dataclass
class Block(Node): stmts: List[Node]
@dataclass
class IfStmt(Node): cond: Node; then: Node; els: Optional[Node]
@dataclass
class WhileStmt(Node): cond: Node; body: Node
@dataclass
class DoWhile(Node): body: Node; cond: Node
@dataclass
class ForStmt(Node): init: Optional[Node]; cond: Optional[Node]; inc: Optional[Node]; body: Node
@dataclass
class ReturnStmt(Node): val: Optional[Node]
@dataclass
class BreakStmt(Node): pass
@dataclass
class ContinueStmt(Node): pass
@dataclass
class ExprStmt(Node): expr: Node
@dataclass
class SwitchStmt(Node): expr: Node; body: Node
@dataclass
class CaseStmt(Node): val: Optional[Node]; stmt: Node  # None = default

# Expressions
@dataclass
class IntLit(Node): val: int; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class StrLit(Node): val: str; ty: CType = field(default_factory=lambda: T_CHARP)
@dataclass
class Var(Node): name: str; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class BinOp(Node): op: str; left: Node; right: Node; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class UnaryOp(Node): op: str; expr: Node; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class Assign(Node): op: str; target: Node; value: Node; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class Call(Node): func: Node; args: List[Node]; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class Index(Node): arr: Node; idx: Node; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class Member(Node): expr: Node; field_name: str; is_ptr: bool; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class Cast(Node): expr: Node; ty: CType
@dataclass
class Sizeof(Node): arg: Any; ty: CType = field(default_factory=lambda: T_LONG)  # CType or Node
@dataclass
class Ternary(Node): cond: Node; then: Node; els: Node; ty: CType = field(default_factory=lambda: T_INT)
@dataclass
class Comma(Node): exprs: List[Node]; ty: CType = field(default_factory=lambda: T_INT)

# ─────────────────────────────────────────────────────────────
# §5  PREPROCESSOR  (simple token-level)
# ─────────────────────────────────────────────────────────────
class Preprocessor:
    """Minimal preprocessor: handles #include (file), #define, #ifdef/#ifndef/#endif"""
    def __init__(self, filename: str, include_dirs=None):
        self.include_dirs = include_dirs or ['/usr/include', '/usr/local/include']
        self.defines: Dict[str,str] = {
            '__ZKAEDI_CC__': '1',
            '__linux__': '1',
            '__x86_64__': '1',
        }
        self.filename = filename
        self.processed_files = set()

    def process_file(self, filename: str) -> str:
        if filename in self.processed_files: return ''
        self.processed_files.add(filename)
        try:
            with open(filename) as f: src = f.read()
        except: return ''
        return self.process(src, filename)

    def process(self, src: str, filename: str) -> str:
        lines = src.split('\n')
        out = []
        i = 0
        ifdef_stack = []  # True = include, False = skip

        def active():
            return all(ifdef_stack) if ifdef_stack else True

        while i < len(lines):
            line = lines[i]; stripped = line.lstrip()
            if stripped.startswith('#'):
                directive = stripped[1:].lstrip()
                # handle line continuations
                while line.endswith('\\') and i+1 < len(lines):
                    i += 1; line = line[:-1] + lines[i]
                    directive = directive[:-1] + lines[i].lstrip() if directive.endswith('\\') else directive
                directive = stripped[1:].lstrip()
                parts = directive.split(None, 1)
                cmd = parts[0] if parts else ''
                rest = parts[1].strip() if len(parts)>1 else ''

                if cmd == 'define' and active():
                    m = re.match(r'(\w+)\s*(.*)', rest)
                    if m: self.defines[m.group(1)] = m.group(2)
                elif cmd == 'undef' and active():
                    self.defines.pop(rest.strip(), None)
                elif cmd == 'ifdef':
                    ifdef_stack.append(rest.strip() in self.defines)
                elif cmd == 'ifndef':
                    ifdef_stack.append(rest.strip() not in self.defines)
                elif cmd == 'if':
                    # very basic: handle #if 0 / #if 1
                    val = rest.strip()
                    ifdef_stack.append(val != '0' and val != '')
                elif cmd == 'elif':
                    if ifdef_stack:
                        prev = ifdef_stack.pop()
                        val = rest.strip()
                        ifdef_stack.append(not prev and val != '0')
                elif cmd == 'else':
                    if ifdef_stack: ifdef_stack[-1] = not ifdef_stack[-1]
                elif cmd == 'endif':
                    if ifdef_stack: ifdef_stack.pop()
                elif cmd == 'include' and active():
                    # Only process local includes (not system)
                    m_local = re.match(r'"([^"]+)"', rest)
                    if m_local:
                        inc_path = os.path.join(os.path.dirname(filename), m_local.group(1))
                        if os.path.exists(inc_path):
                            out.append(self.process_file(inc_path))
                    # System includes: emit as comment (we rely on stdlib)
                    # out.append(f"/* #include {rest} */")
                elif cmd == 'pragma' and active():
                    pass  # ignore
                out.append(f'/* #{cmd} */ ')
            elif active():
                # Apply simple define substitutions (whole words only)
                processed = line
                for k,v in self.defines.items():
                    if v:  # skip empty defines
                        processed = re.sub(r'\b' + re.escape(k) + r'\b', v, processed)
                out.append(processed)
            else:
                out.append('')
            i += 1

        return '\n'.join(out)

# ─────────────────────────────────────────────────────────────
# §6  PARSER
# ─────────────────────────────────────────────────────────────
class Parser:
    def __init__(self, tokens: List[Token], filename="<stdin>"):
        self.tokens = tokens
        self.pos = 0
        self.filename = filename
        # struct registry: tag -> CType
        self.structs: Dict[str, CType] = {}
        # typedef registry
        self.typedefs: Dict[str, CType] = {}
        # global declarations
        self.decls: List[Node] = []

    def _cur(self) -> Token: return self.tokens[self.pos]
    def _peek(self, n=1) -> Token:
        p = self.pos+n
        return self.tokens[p] if p<len(self.tokens) else self.tokens[-1]

    def _advance(self) -> Token:
        t = self.tokens[self.pos]
        if self.pos < len(self.tokens)-1: self.pos += 1
        return t

    def _expect(self, kind, val=None) -> Token:
        t = self._cur()
        if t.kind != kind or (val is not None and t.val != val):
            self._error(f"expected {kind.name}({val!r}) got {t.kind.name}({t.val!r})")
        return self._advance()

    def _check(self, kind, val=None) -> bool:
        t = self._cur()
        if t.kind != kind: return False
        if val is not None and t.val != val: return False
        return True

    def _match(self, kind, val=None) -> bool:
        if self._check(kind, val): self._advance(); return True
        return False

    def _error(self, msg):
        t = self._cur()
        raise SyntaxError(f"{self.filename}:{t.line}:{t.col}: {msg}")

    def _is_type_start(self) -> bool:
        t = self._cur()
        if t.kind in (TK.VOID,TK.CHAR,TK.INT,TK.LONG,TK.SHORT,TK.SIGNED,TK.UNSIGNED,
                      TK.STRUCT,TK.UNION,TK.CONST,TK.VOLATILE,TK.STATIC,TK.EXTERN,
                      TK.INLINE,TK.TYPEDEF,TK.AUTO,TK.REGISTER): return True
        if t.kind == TK.IDENT and t.val in self.typedefs: return True
        return False

    # ── Type parsing ──────────────────────────────────────────

    def _parse_type_specifiers(self):
        """Returns (base_type, storage_class, is_const)"""
        storage = None; is_const = False; is_inline = False
        base = None
        unsigned = False; signed = False; long_count = 0; short = False

        while True:
            t = self._cur()
            if t.kind == TK.CONST: self._advance(); is_const = True
            elif t.kind == TK.VOLATILE: self._advance()  # ignore
            elif t.kind == TK.STATIC: self._advance(); storage = 'static'
            elif t.kind == TK.EXTERN: self._advance(); storage = 'extern'
            elif t.kind == TK.INLINE: self._advance(); is_inline = True
            elif t.kind == TK.AUTO or t.kind == TK.REGISTER: self._advance()
            elif t.kind == TK.TYPEDEF: self._advance(); storage = 'typedef'
            elif t.kind == TK.UNSIGNED: self._advance(); unsigned = True
            elif t.kind == TK.SIGNED: self._advance(); signed = True
            elif t.kind == TK.LONG: self._advance(); long_count += 1
            elif t.kind == TK.SHORT: self._advance(); short = True
            elif t.kind == TK.VOID: self._advance(); base = T_VOID
            elif t.kind == TK.CHAR: self._advance(); base = T_CHAR
            elif t.kind == TK.INT: self._advance(); base = T_INT
            elif t.kind == TK.DOUBLE or t.kind == TK.FLOAT:
                self._advance(); base = T_LONG  # treat float/double as long
            elif t.kind == TK.STRUCT or t.kind == TK.UNION:
                base = self._parse_struct_or_union(); break
            elif t.kind == TK.ENUM:
                base = self._parse_enum(); break
            elif t.kind == TK.IDENT and t.val in self.typedefs:
                self._advance(); base = self.typedefs[t.val]; break
            else: break

        if base is None:
            if long_count > 0 or short: base = T_INT
            else: base = T_INT  # default int

        # apply unsigned/long
        if unsigned:
            if base.kind == TypeKind.CHAR: base = T_UCHAR
            elif base.kind == TypeKind.INT:
                base = T_UINT if not (long_count>0) else T_ULONG
            else: base = T_ULONG
        elif long_count > 0 and base.kind == TypeKind.INT:
            base = T_LONG

        if is_const: base = CType(base.kind, base.base, base.array_size, base.struct_tag, base.members, base.struct_size, base.params, base.ret, base.variadic, True)
        return base, storage

    def _parse_struct_or_union(self) -> CType:
        is_union = self._cur().kind == TK.UNION
        self._advance()  # consume struct/union
        tag = ''
        if self._cur().kind == TK.IDENT:
            tag = self._cur().val; self._advance()

        if self._cur().kind == TK.PUNCT and self._cur().val == '{':
            self._advance()
            members = []
            offset = 0
            while not (self._check(TK.PUNCT, '}')):
                mbase, _ = self._parse_type_specifiers()
                # parse possibly multiple declarators
                while True:
                    mname, mtype = self._parse_declarator(mbase)
                    # align
                    al = mtype.align()
                    if al > 0: offset = (offset + al - 1) & ~(al-1)
                    members.append((mname, mtype, 0 if is_union else offset))
                    if not is_union: offset += mtype.size()
                    if self._check(TK.PUNCT, ','): self._advance()
                    else: break
                self._expect(TK.PUNCT, ';')
            self._expect(TK.PUNCT, '}')
            total = 0 if is_union else offset
            if is_union: total = max((m[1].size() for m in members), default=0)
            al = max((m[1].align() for m in members), default=1)
            total = (total + al - 1) & ~(al-1)
            ty = CType(TypeKind.STRUCT, struct_tag=tag, members=members, struct_size=total)
            if tag: self.structs[tag] = ty
            return ty
        else:
            # reference to existing struct
            if tag and tag in self.structs: return self.structs[tag]
            ty = CType(TypeKind.STRUCT, struct_tag=tag, members=[], struct_size=0)
            if tag: self.structs[tag] = ty
            return ty

    def _parse_enum(self) -> CType:
        self._advance()  # consume 'enum'
        if self._cur().kind == TK.IDENT: self._advance()
        if self._check(TK.PUNCT, '{'):
            self._advance()
            val = 0
            while not self._check(TK.PUNCT, '}'):
                name = self._expect(TK.IDENT).val
                if self._check(TK.PUNCT, '='):
                    self._advance(); val = self._const_int_expr()
                self.typedefs[name] = T_INT
                # register as enum constant - we'll handle via typedefs mapping to value
                # For simplicity, we emit them as globals
                val += 1
                if self._check(TK.PUNCT, ','): self._advance()
            self._expect(TK.PUNCT, '}')
        return T_INT

    def _const_int_expr(self) -> int:
        """Evaluate a simple constant integer expression"""
        return self._parse_cond_const()

    def _parse_cond_const(self) -> int:
        v = self._parse_lor_const()
        if self._check(TK.PUNCT, '?'):
            self._advance()
            t = self._parse_cond_const()
            self._expect(TK.PUNCT, ':')
            f = self._parse_cond_const()
            return t if v else f
        return v

    def _parse_lor_const(self) -> int:
        v = self._parse_primary_const()
        while self._check(TK.LOR): self._advance(); r=self._parse_primary_const(); v=v or r
        return v

    def _parse_primary_const(self) -> int:
        t = self._cur()
        if t.kind == TK.INT_LIT: self._advance(); return t.val
        if t.kind == TK.LONG_LIT: self._advance(); return t.val
        if t.kind == TK.CHAR_LIT: self._advance(); return t.val
        if t.kind == TK.IDENT and t.val in self.typedefs:
            self._advance(); return 0  # unknown
        if t.kind == TK.PUNCT and t.val == '-':
            self._advance(); return -self._parse_primary_const()
        if t.kind == TK.PUNCT and t.val == '(':
            self._advance(); v=self._parse_cond_const(); self._expect(TK.PUNCT,')'); return v
        self._advance(); return 0

    def _parse_declarator(self, base: CType):
        """Parse declarator, return (name, type). Handles *, [], ()"""
        # Count pointer stars (with optional const/volatile)
        stars = 0
        while self._check(TK.PUNCT, '*') or self._cur().kind in (TK.CONST, TK.VOLATILE):
            if self._cur().kind in (TK.CONST, TK.VOLATILE): self._advance(); continue
            self._advance(); stars += 1

        # Direct declarator: name, or ( declarator )
        name = ''
        if self._cur().kind == TK.IDENT: name = self._advance().val
        elif self._check(TK.PUNCT, '('):
            # function pointer or grouped declarator - parse inner then come back
            self._advance()
            inner_name, inner_type = self._parse_declarator(base)
            self._expect(TK.PUNCT, ')')
            name = inner_name
            # Optional * and const/volatile after ) e.g. "void (*)(int)" or "( ) *"
            more_stars = 0
            while self._check(TK.PUNCT, '*') or self._cur().kind in (TK.CONST, TK.VOLATILE):
                if self._cur().kind in (TK.CONST, TK.VOLATILE): self._advance(); continue
                self._advance(); more_stars += 1
            ty = inner_type
            for _ in range(more_stars): ty = ptr_to(ty)
            for _ in range(stars): ty = ptr_to(ty)
            ty = self._parse_declarator_suffix(ty)
            return name, ty

        # Suffix: [] or ()
        ty = self._parse_declarator_suffix(base)

        # Apply pointers (outermost pointer is last star)
        for _ in range(stars): ty = ptr_to(ty)

        return name, ty

    def _parse_declarator_suffix(self, base: CType) -> CType:
        """Handle [] and () suffixes"""
        if self._check(TK.PUNCT, '['):
            sizes = []
            while self._check(TK.PUNCT, '['):
                self._advance()
                if self._check(TK.PUNCT, ']'): sizes.append(0); self._advance()
                else: sizes.append(self._const_int_expr()); self._expect(TK.PUNCT, ']')
            ty = base
            for sz in reversed(sizes): ty = arr_of(ty, sz)
            return ty
        elif self._check(TK.PUNCT, '('):
            # function type (in declaration context)
            return base  # simplified
        return base

    # ── Translation unit ──────────────────────────────────────

    def parse(self) -> TranslationUnit:
        decls = []
        while not self._check(TK.EOF):
            d = self._parse_external_decl()
            if d is not None:
                if isinstance(d, list): decls.extend(d)
                else: decls.append(d)
        return TranslationUnit(decls)

    def _parse_external_decl(self):
        # typedef?
        if self._check(TK.TYPEDEF):
            return self._parse_typedef()

        base, storage = self._parse_type_specifiers()

        # struct/union/enum definition only (no declarator)
        if self._check(TK.PUNCT, ';'):
            self._advance(); return None

        # Declarator
        name, ty = self._parse_declarator(base)

        if not name:
            self._advance(); return None

        # Function definition or declaration?
        if self._check(TK.PUNCT, '(') or (ty.kind == TypeKind.FUNC):
            return self._parse_func_decl(name, base, storage)

        # It's a (possibly function) declarator followed by ( -> function
        # Check if it's a function: look for (
        if self._check(TK.PUNCT, '('):
            return self._parse_func_decl(name, base, storage)

        # Global variable(s)
        decls = []
        while True:
            init = None
            if self._check(TK.PUNCT, '='):
                self._advance(); init = self._parse_initializer(ty)
            decls.append(VarDecl(name, ty, init, is_global=True,
                                 is_static=(storage=='static'),
                                 is_extern=(storage=='extern')))
            if not self._check(TK.PUNCT, ','): break
            self._advance()
            name, ty = self._parse_declarator(base)
        self._expect(TK.PUNCT, ';')
        return decls if len(decls)>1 else decls[0]

    def _parse_typedef(self):
        self._advance()  # consume typedef
        base, _ = self._parse_type_specifiers()
        while True:
            name, ty = self._parse_declarator(base)
            if name: self.typedefs[name] = ty
            if not self._check(TK.PUNCT, ','): break
            self._advance()
        self._expect(TK.PUNCT, ';')
        return None

    def _parse_func_decl(self, name, ret_base, storage):
        self._expect(TK.PUNCT, '(')
        params = []; variadic = False
        if not self._check(TK.PUNCT, ')'):
            if self._check(TK.ELLIPSIS):
                self._advance(); variadic = True
            else:
                while True:
                    if self._check(TK.ELLIPSIS):
                        self._advance(); variadic = True; break
                    pbase, _ = self._parse_type_specifiers()
                    pname, pty = self._parse_declarator(pbase)
                    # void as sole param means no params
                    if pty.kind == TypeKind.VOID and not pname:
                        break
                    # Decay array to pointer
                    if pty.kind == TypeKind.ARRAY:
                        pty = ptr_to(pty.base)
                    params.append((pname or '', pty))
                    if not self._check(TK.PUNCT, ','): break
                    self._advance()
        self._expect(TK.PUNCT, ')')

        # K&R style param declarations
        while self._is_type_start() and not self._check(TK.PUNCT, '{'):
            # skip K&R declarations
            pbase, _ = self._parse_type_specifiers()
            while True:
                pn, pt = self._parse_declarator(pbase)
                if pn:
                    for i,(n2,_) in enumerate(params):
                        if n2 == pn: params[i] = (pn, pt)
                if not self._check(TK.PUNCT, ','): break
                self._advance()
            self._expect(TK.PUNCT, ';')

        body = None
        if self._check(TK.PUNCT, '{'):
            body = self._parse_block()
        else:
            self._expect(TK.PUNCT, ';')

        return FuncDecl(name, ret_base, params, body,
                        is_static=(storage=='static'),
                        is_extern=(storage=='extern' or body is None))

    def _parse_initializer(self, ty: CType):
        if self._check(TK.PUNCT, '{'):
            self._advance()
            # Array/struct initializer
            items = []
            while not self._check(TK.PUNCT, '}'):
                if self._check(TK.PUNCT, '.'):
                    # designated initializer - skip designator
                    self._advance(); self._expect(TK.IDENT)
                    self._expect(TK.PUNCT, '=')
                items.append(self._parse_assign_expr())
                if not self._check(TK.PUNCT, ','): break
                self._advance()
            self._expect(TK.PUNCT, '}')
            return BinOp('{}', IntLit(len(items)), Block(items))  # reuse as container
        return self._parse_assign_expr()

    # ── Statement parsing ──────────────────────────────────────

    def _parse_block(self) -> Block:
        self._expect(TK.PUNCT, '{')
        stmts = []
        while not self._check(TK.PUNCT, '}') and not self._check(TK.EOF):
            stmts.append(self._parse_block_item())
        self._expect(TK.PUNCT, '}')
        return Block(stmts)

    def _parse_block_item(self):
        if self._check(TK.TYPEDEF):
            self._parse_typedef(); return Block([])
        if self._is_type_start():
            return self._parse_local_decls()
        return self._parse_stmt()

    def _parse_local_decls(self):
        base, storage = self._parse_type_specifiers()
        decls = []
        while True:
            name, ty = self._parse_declarator(base)
            if not name:
                # Might be anonymous struct/union definition
                self._expect(TK.PUNCT, ';'); return Block(decls)
            init = None
            if self._check(TK.PUNCT, '='):
                self._advance(); init = self._parse_initializer(ty)
            decls.append(VarDecl(name, ty, init, is_global=False,
                                 is_static=(storage=='static')))
            if not self._check(TK.PUNCT, ','): break
            self._advance()
        self._expect(TK.PUNCT, ';')
        return Block(decls) if len(decls)>1 else decls[0]

    def _parse_stmt(self):
        t = self._cur()
        if t.kind == TK.IF: return self._parse_if()
        if t.kind == TK.WHILE: return self._parse_while()
        if t.kind == TK.DO: return self._parse_do_while()
        if t.kind == TK.FOR: return self._parse_for()
        if t.kind == TK.RETURN:
            self._advance()
            if self._check(TK.PUNCT, ';'): self._advance(); return ReturnStmt(None)
            v = self._parse_expr(); self._expect(TK.PUNCT, ';'); return ReturnStmt(v)
        if t.kind == TK.BREAK: self._advance(); self._expect(TK.PUNCT,';'); return BreakStmt()
        if t.kind == TK.CONTINUE: self._advance(); self._expect(TK.PUNCT,';'); return ContinueStmt()
        if t.kind == TK.SWITCH: return self._parse_switch()
        if t.kind == TK.CASE or t.kind == TK.DEFAULT:
            return self._parse_case()
        if t.kind == TK.GOTO:
            self._advance(); self._expect(TK.IDENT); self._expect(TK.PUNCT,';')
            return Block([])  # ignore goto for now
        if t.kind == TK.PUNCT and t.val == '{': return self._parse_block()
        if t.kind == TK.PUNCT and t.val == ';': self._advance(); return Block([])
        # label?
        if t.kind == TK.IDENT and self._peek().kind == TK.PUNCT and self._peek().val == ':':
            self._advance(); self._advance()  # consume label:
            return self._parse_stmt()
        # Expression statement
        e = self._parse_expr()
        self._expect(TK.PUNCT, ';')
        return ExprStmt(e)

    def _parse_if(self):
        self._advance()  # if
        self._expect(TK.PUNCT,'('); cond=self._parse_expr(); self._expect(TK.PUNCT,')')
        then = self._parse_stmt()
        els = None
        if self._match(TK.ELSE): els = self._parse_stmt()
        return IfStmt(cond, then, els)

    def _parse_while(self):
        self._advance()
        self._expect(TK.PUNCT,'('); cond=self._parse_expr(); self._expect(TK.PUNCT,')')
        return WhileStmt(cond, self._parse_stmt())

    def _parse_do_while(self):
        self._advance()
        body = self._parse_stmt()
        self._expect(TK.WHILE); self._expect(TK.PUNCT,'(')
        cond = self._parse_expr(); self._expect(TK.PUNCT,')'); self._expect(TK.PUNCT,';')
        return DoWhile(body, cond)

    def _parse_for(self):
        self._advance(); self._expect(TK.PUNCT,'(')
        init = None
        if not self._check(TK.PUNCT,';'):
            if self._is_type_start(): init = self._parse_local_decls()
            else: e=self._parse_expr(); self._expect(TK.PUNCT,';'); init=ExprStmt(e)
        else: self._advance()
        cond = None
        if not self._check(TK.PUNCT,';'): cond = self._parse_expr()
        self._expect(TK.PUNCT,';')
        inc = None
        if not self._check(TK.PUNCT,')'): inc = self._parse_expr()
        self._expect(TK.PUNCT,')')
        return ForStmt(init, cond, inc, self._parse_stmt())

    def _parse_switch(self):
        self._advance(); self._expect(TK.PUNCT,'(')
        e = self._parse_expr(); self._expect(TK.PUNCT,')')
        body = self._parse_stmt()
        return SwitchStmt(e, body)

    def _parse_case(self):
        t = self._cur()
        if t.kind == TK.DEFAULT:
            self._advance(); self._expect(TK.PUNCT,':')
            return CaseStmt(None, self._parse_stmt())
        self._advance()  # case
        val = self._parse_cond_expr()
        self._expect(TK.PUNCT,':')
        return CaseStmt(val, self._parse_stmt())

    # ── Expression parsing ─────────────────────────────────────

    def _parse_expr(self):
        e = self._parse_assign_expr()
        if self._check(TK.PUNCT, ','):
            exprs = [e]
            while self._check(TK.PUNCT, ','):
                self._advance(); exprs.append(self._parse_assign_expr())
            return Comma(exprs)
        return e

    def _parse_assign_expr(self):
        return self._parse_assign_rhs(self._parse_cond_expr())

    def _parse_assign_rhs(self, left):
        ops = {TK.PUNCT: ['='], TK.ADD_ASSIGN:['+='], TK.SUB_ASSIGN:['-='],
               TK.MUL_ASSIGN:['*='], TK.DIV_ASSIGN:['/='], TK.MOD_ASSIGN:['%='],
               TK.AND_ASSIGN:['&='], TK.OR_ASSIGN:['|='], TK.XOR_ASSIGN:['^='],
               TK.LSHIFT_ASSIGN:['<<='], TK.RSHIFT_ASSIGN:['>>='],}
        t = self._cur()
        for k,vs in ops.items():
            if t.kind == k and (k != TK.PUNCT or t.val in vs):
                op = t.val; self._advance()
                right = self._parse_assign_expr()
                return Assign(op, left, right)
        return left

    def _parse_cond_expr(self):
        e = self._parse_lor_expr()
        if self._check(TK.PUNCT, '?'):
            self._advance()
            then = self._parse_expr()
            self._expect(TK.PUNCT, ':')
            els = self._parse_cond_expr()
            return Ternary(e, then, els)
        return e

    def _parse_lor_expr(self):
        e = self._parse_land_expr()
        while self._check(TK.LOR): self._advance(); r=self._parse_land_expr(); e=BinOp('||',e,r)
        return e

    def _parse_land_expr(self):
        e = self._parse_bor_expr()
        while self._check(TK.LAND): self._advance(); r=self._parse_bor_expr(); e=BinOp('&&',e,r)
        return e

    def _parse_bor_expr(self):
        e = self._parse_xor_expr()
        while self._check(TK.PUNCT,'|'): self._advance(); r=self._parse_xor_expr(); e=BinOp('|',e,r)
        return e

    def _parse_xor_expr(self):
        e = self._parse_band_expr()
        while self._check(TK.PUNCT,'^'): self._advance(); r=self._parse_band_expr(); e=BinOp('^',e,r)
        return e

    def _parse_band_expr(self):
        e = self._parse_eq_expr()
        while self._check(TK.PUNCT,'&'): self._advance(); r=self._parse_eq_expr(); e=BinOp('&',e,r)
        return e

    def _parse_eq_expr(self):
        e = self._parse_rel_expr()
        while True:
            if self._check(TK.EQ): self._advance(); r=self._parse_rel_expr(); e=BinOp('==',e,r)
            elif self._check(TK.NEQ): self._advance(); r=self._parse_rel_expr(); e=BinOp('!=',e,r)
            else: break
        return e

    def _parse_rel_expr(self):
        e = self._parse_shift_expr()
        while True:
            if self._check(TK.PUNCT,'<') and not self._check(TK.LSHIFT_ASSIGN):
                self._advance(); r=self._parse_shift_expr(); e=BinOp('<',e,r)
            elif self._check(TK.PUNCT,'>') and not self._check(TK.RSHIFT_ASSIGN):
                self._advance(); r=self._parse_shift_expr(); e=BinOp('>',e,r)
            elif self._check(TK.LEQ): self._advance(); r=self._parse_shift_expr(); e=BinOp('<=',e,r)
            elif self._check(TK.GEQ): self._advance(); r=self._parse_shift_expr(); e=BinOp('>=',e,r)
            else: break
        return e

    def _parse_shift_expr(self):
        e = self._parse_add_expr()
        while True:
            if self._check(TK.LSHIFT): self._advance(); r=self._parse_add_expr(); e=BinOp('<<',e,r)
            elif self._check(TK.RSHIFT): self._advance(); r=self._parse_add_expr(); e=BinOp('>>',e,r)
            else: break
        return e

    def _parse_add_expr(self):
        e = self._parse_mul_expr()
        while True:
            if self._check(TK.PUNCT,'+'): self._advance(); r=self._parse_mul_expr(); e=BinOp('+',e,r)
            elif self._check(TK.PUNCT,'-'): self._advance(); r=self._parse_mul_expr(); e=BinOp('-',e,r)
            else: break
        return e

    def _parse_mul_expr(self):
        e = self._parse_cast_expr()
        while True:
            if self._check(TK.PUNCT,'*'): self._advance(); r=self._parse_cast_expr(); e=BinOp('*',e,r)
            elif self._check(TK.PUNCT,'/'): self._advance(); r=self._parse_cast_expr(); e=BinOp('/',e,r)
            elif self._check(TK.PUNCT,'%'): self._advance(); r=self._parse_cast_expr(); e=BinOp('%',e,r)
            else: break
        return e

    def _parse_cast_expr(self):
        # Try to detect cast: (type) expr
        if self._check(TK.PUNCT,'('):
            saved_pos = self.pos
            try:
                self._advance()
                if self._is_type_start():
                    base, _ = self._parse_type_specifiers()
                    _, ty = self._parse_declarator(base)
                    if self._check(TK.PUNCT,')'):
                        self._advance()
                        e = self._parse_cast_expr()
                        return Cast(e, ty)
            except: pass
            self.pos = saved_pos
        return self._parse_unary_expr()

    def _parse_unary_expr(self):
        t = self._cur()
        if t.kind == TK.INC: self._advance(); e=self._parse_unary_expr(); return UnaryOp('pre++',e)
        if t.kind == TK.DEC: self._advance(); e=self._parse_unary_expr(); return UnaryOp('pre--',e)
        if t.kind == TK.PUNCT:
            if t.val == '&': self._advance(); e=self._parse_cast_expr(); return UnaryOp('&',e)
            if t.val == '*': self._advance(); e=self._parse_cast_expr(); return UnaryOp('*',e)
            if t.val == '-': self._advance(); e=self._parse_cast_expr(); return UnaryOp('-',e)
            if t.val == '+': self._advance(); return self._parse_cast_expr()
            if t.val == '~': self._advance(); e=self._parse_cast_expr(); return UnaryOp('~',e)
            if t.val == '!': self._advance(); e=self._parse_cast_expr(); return UnaryOp('!',e)
        if t.kind == TK.SIZEOF: return self._parse_sizeof()
        return self._parse_postfix_expr()

    def _parse_sizeof(self):
        self._advance()
        if self._check(TK.PUNCT,'('):
            saved = self.pos
            self._advance()
            if self._is_type_start():
                base,_=self._parse_type_specifiers()
                _,ty=self._parse_declarator(base)
                if self._check(TK.PUNCT,')'):
                    self._advance(); return Sizeof(ty)
            self.pos = saved
        e = self._parse_unary_expr()
        return Sizeof(e)

    def _parse_postfix_expr(self):
        e = self._parse_primary_expr()
        while True:
            t = self._cur()
            if t.kind == TK.PUNCT and t.val == '[':
                self._advance(); idx=self._parse_expr(); self._expect(TK.PUNCT,']')
                e = Index(e, idx)
            elif t.kind == TK.PUNCT and t.val == '(':
                self._advance(); args=[]
                if not self._check(TK.PUNCT,')'):
                    args.append(self._parse_assign_expr())
                    while self._check(TK.PUNCT,','):
                        self._advance(); args.append(self._parse_assign_expr())
                self._expect(TK.PUNCT,')')
                e = Call(e, args)
            elif t.kind == TK.PUNCT and t.val == '.':
                self._advance(); fname=self._expect(TK.IDENT).val
                e = Member(e, fname, False)
            elif t.kind == TK.ARROW:
                self._advance(); fname=self._expect(TK.IDENT).val
                e = Member(e, fname, True)
            elif t.kind == TK.INC: self._advance(); e=UnaryOp('post++',e)
            elif t.kind == TK.DEC: self._advance(); e=UnaryOp('post--',e)
            else: break
        return e

    def _parse_primary_expr(self):
        t = self._cur()
        if t.kind in (TK.INT_LIT, TK.CHAR_LIT):
            self._advance(); return IntLit(t.val, T_INT)
        if t.kind == TK.LONG_LIT:
            self._advance(); return IntLit(t.val, T_LONG)
        if t.kind == TK.STR_LIT:
            # Concatenate adjacent string literals
            s = t.val; self._advance()
            while self._cur().kind == TK.STR_LIT:
                s += self._cur().val; self._advance()
            return StrLit(s)
        if t.kind == TK.IDENT:
            self._advance(); return Var(t.val)
        if t.kind == TK.PUNCT and t.val == '(':
            self._advance(); e=self._parse_expr(); self._expect(TK.PUNCT,')'); return e
        self._error(f"unexpected token {t!r}")

# ─────────────────────────────────────────────────────────────
# §7  SYMBOL TABLE + TYPE RESOLUTION
# ─────────────────────────────────────────────────────────────
@dataclass
class Symbol:
    name: str
    ty: CType
    is_global: bool
    offset: int = 0      # rbp offset for locals; 0 for globals
    label: str = ''      # asm label for globals/strings
    is_param: bool = False
    is_static_local: bool = False
    param_reg_idx: int = -1  # which param register

class Scope:
    def __init__(self, parent=None):
        self.parent = parent
        self.syms: Dict[str, Symbol] = {}
    def define(self, sym: Symbol): self.syms[sym.name] = sym
    def lookup(self, name: str) -> Optional[Symbol]:
        if name in self.syms: return self.syms[name]
        if self.parent: return self.parent.lookup(name)
        return None

# ─────────────────────────────────────────────────────────────
# §8  CODE GENERATOR  (x86-64 AT&T syntax → GAS)
# ─────────────────────────────────────────────────────────────
PARAM_REGS_64 = ['%rdi','%rsi','%rdx','%rcx','%r8','%r9']
PARAM_REGS_32 = ['%edi','%esi','%edx','%ecx','%r8d','%r9d']

class CodeGen:
    def __init__(self, filename="<stdin>"):
        self.filename = filename
        self.out: List[str] = []
        self.label_cnt = 0
        self.str_lits: List[Tuple[str,str]] = []  # (label, value)
        self.global_scope = Scope()
        self.scope = self.global_scope
        self.cur_func: Optional[str] = None
        self.stack_size = 0          # current frame size
        self.local_offset = 0        # next local offset (negative)
        self.break_labels: List[str] = []
        self.continue_labels: List[str] = []
        self.switch_labels: List[str] = []
        self.func_ret_label = ''
        self.func_ret_type: CType = T_INT
        # static local counter
        self.static_cnt = 0
        # enum values
        self.enum_vals: Dict[str, int] = {}
        # function signatures (for call type resolution)
        self.func_sigs: Dict[str, Tuple[CType,List[CType],bool]] = {}

    def _new_label(self, prefix='.L') -> str:
        self.label_cnt += 1
        return f"{prefix}{self.label_cnt}"

    def _emit(self, s): self.out.append(s)
    def _ins(self, s): self.out.append(f'\t{s}')

    def get_assembly(self) -> str:
        return '\n'.join(self.out) + '\n'

    def _type_of(self, node: Node) -> CType:
        if isinstance(node, IntLit): return node.ty
        if isinstance(node, StrLit): return T_CHARP
        if isinstance(node, Var):
            sym = self.scope.lookup(node.name)
            if sym: return sym.ty
            return T_INT
        if isinstance(node, BinOp): return node.ty
        if isinstance(node, UnaryOp): return node.ty
        if isinstance(node, Assign): return node.ty
        if isinstance(node, Call): return node.ty
        if isinstance(node, Cast): return node.ty
        if isinstance(node, Sizeof): return T_LONG
        if isinstance(node, Ternary): return node.ty
        if isinstance(node, Index): return node.ty
        if isinstance(node, Member): return node.ty
        if isinstance(node, Comma): return node.ty
        return T_INT

    def _size_suf(self, sz: int) -> str:
        return {1:'b',2:'w',4:'l',8:'q'}.get(sz,'q')

    def _reg_for_size(self, sz: int, base='%rax') -> str:
        regs = {'%rax':{1:'%al',2:'%ax',4:'%eax',8:'%rax'},
                '%rcx':{1:'%cl',2:'%cx',4:'%ecx',8:'%rcx'},
                '%rdx':{1:'%dl',2:'%dx',4:'%edx',8:'%rdx'},
                '%rdi':{1:'%dil',2:'%di',4:'%edi',8:'%rdi'},
                '%rsi':{1:'%sil',2:'%si',4:'%esi',8:'%rsi'},}
        return regs.get(base, {}).get(sz, base)

    # ── Emit data section ──────────────────────────────────────

    def emit_string_literals(self):
        if not self.str_lits: return
        self._emit('\t.section .rodata')
        for label, val in self.str_lits:
            self._emit(f'{label}:')
            # emit as .string
            escaped = val.replace('\\','\\\\').replace('"','\\"').replace('\n','\\n').replace('\t','\\t').replace('\r','\\r').replace('\0','\\000')
            self._emit(f'\t.string "{escaped}"')

    # ── Top-level generation ───────────────────────────────────

    def generate(self, tu: TranslationUnit):
        self._emit('\t.file "' + self.filename + '"')
        # First pass: collect function signatures
        for d in tu.decls:
            if isinstance(d, FuncDecl):
                ptypes = [pt for _,pt in d.params]
                self.func_sigs[d.name] = (d.ret, ptypes, False)
        # Generate
        for d in tu.decls:
            self._gen_toplevel(d)
        # String literals
        self.emit_string_literals()
        # GNU stack note
        self._emit('\t.section .note.GNU-stack,"",@progbits')

    def _gen_toplevel(self, node: Node):
        if node is None: return
        if isinstance(node, FuncDecl):
            if node.body is not None: self._gen_func(node)
            elif not node.is_extern:
                # Forward declaration - declare as extern
                pass
        elif isinstance(node, VarDecl):
            self._gen_global_var(node)
        elif isinstance(node, Block):
            for s in node.stmts: self._gen_toplevel(s)

    def _gen_global_var(self, node: VarDecl):
        if node.is_extern: return  # just a declaration
        name = node.name
        ty = node.ty
        sz = ty.size()
        if sz == 0: sz = 8  # incomplete

        self.global_scope.define(Symbol(name, ty, True, label=name))

        if node.init is None:
            if node.is_static:
                self._emit(f'\t.local {name}')
            else:
                self._emit(f'\t.comm {name},{sz},{ty.align()}')
            return

        if node.is_static:
            self._emit(f'\t.local {name}')
        else:
            self._emit(f'\t.globl {name}')
        al = ty.align()
        self._emit(f'\t.data')
        if al > 1: self._emit(f'\t.align {al}')
        self._emit(f'\t.type {name},@object')
        self._emit(f'\t.size {name},{sz}')
        self._emit(f'{name}:')
        self._gen_global_init(ty, node.init)

    def _gen_global_init(self, ty: CType, init: Node):
        sz = ty.size()
        if isinstance(init, IntLit):
            self._emit(f'\t.{self._data_directive(sz)} {init.val}')
        elif isinstance(init, StrLit):
            label = self._new_label('.LC')
            self.str_lits.append((label, init.val))
            self._emit(f'\t.quad {label}')
        elif isinstance(init, UnaryOp) and init.op == '&':
            if isinstance(init.expr, Var):
                self._emit(f'\t.quad {init.expr.name}')
            else: self._emit(f'\t.zero {sz}')
        elif isinstance(init, BinOp) and init.op == '{}':
            # array/struct initializer
            items = init.right.stmts if isinstance(init.right, Block) else []
            count = init.left.val if isinstance(init.left, IntLit) else 0
            elem_ty = ty.base if ty.kind == TypeKind.ARRAY else T_INT
            for item in items:
                self._gen_global_init(elem_ty, item)
            # zero-fill remainder
            emitted = len(items) * elem_ty.size()
            if sz > emitted: self._emit(f'\t.zero {sz-emitted}')
        else:
            self._emit(f'\t.zero {sz}')

    def _data_directive(self, sz: int) -> str:
        return {1:'byte',2:'value',4:'long',8:'quad'}.get(sz,'quad')

    # ── Function code generation ───────────────────────────────

    def _gen_func(self, node: FuncDecl):
        name = node.name
        self.cur_func = name
        self.func_ret_type = node.ret
        self.local_offset = 0

        # Emit function prologue
        if not node.is_static:
            self._emit(f'\t.globl {name}')
        self._emit(f'\t.type {name},@function')
        self._emit(f'{name}:')
        self._emit(f'\t.cfi_startproc')

        # Save rbp, set up frame
        self._ins('pushq %rbp')
        self._emit('\t.cfi_def_cfa_offset 16')
        self._ins('movq %rsp, %rbp')
        self._emit('\t.cfi_offset %rbp, -16')
        self._emit('\t.cfi_def_cfa_register %rbp')

        # Reserve space for locals - we'll fix this up later
        stack_reserve_pos = len(self.out)
        self._ins('subq $STACK_SIZE, %rsp')  # placeholder

        # New scope for function
        self.scope = Scope(self.global_scope)

        # Save callee-saved registers we'll use (r12, r13 for loop vars)
        self._ins('pushq %r12')
        self._ins('pushq %r13')
        self._ins('pushq %r14')
        self._ins('pushq %r15')

        # Spill parameter registers to stack
        for i, (pname, pty) in enumerate(node.params):
            if not pname: continue
            self.local_offset -= 8  # align to 8 bytes for simplicity
            offset = self.local_offset
            sym = Symbol(pname, pty, False, offset=offset, is_param=True)
            self.scope.define(sym)
            sz = pty.size()
            if i < 6:
                preg = PARAM_REGS_64[i]
                if sz <= 4:
                    sreg = PARAM_REGS_32[i]
                    self._ins(f'movl {sreg}, {offset}(%rbp)')
                else:
                    self._ins(f'movq {preg}, {offset}(%rbp)')
            else:
                # Parameter passed on stack: at rbp+16, rbp+24, ...
                stack_off = 16 + (i-6)*8
                self._ins(f'movq {stack_off}(%rbp), %rax')
                self._ins(f'movq %rax, {offset}(%rbp)')

        # Return label
        self.func_ret_label = self._new_label('.Lret')

        # Generate body
        self._gen_stmt(node.body)

        # Return label
        self._emit(f'{self.func_ret_label}:')

        # Epilogue
        self._ins('popq %r15')
        self._ins('popq %r14')
        self._ins('popq %r13')
        self._ins('popq %r12')
        self._ins('movq %rbp, %rsp')
        self._ins('popq %rbp')
        self._emit('\t.cfi_def_cfa %rsp, 8')
        self._ins('ret')
        self._emit('\t.cfi_endproc')
        sz_label = self._new_label('.Lfe')
        self._emit(f'\t.size {name}, .-{name}')

        # Fix up stack reservation
        # local_offset is negative, abs value is bytes needed (plus callee-saved regs)
        frame_size = (-self.local_offset + 7) & ~7
        # Make 16-byte aligned after push rbp + 4 callee regs = 5 pushes = 40 bytes
        # rsp after prologue pushes = original-8 (rbp) - 8*4 (callee) = -40
        # We need (frame_size + 40) to be multiple of 16, so frame_size % 16 == 8
        # Simplify: round up to next multiple of 16
        frame_size = ((frame_size + 15) & ~15)
        self.out[stack_reserve_pos] = f'\tsubq ${frame_size}, %rsp'

        self.scope = self.global_scope
        self.cur_func = None

    # ── Statement generation ────────────────────────────────────

    def _gen_stmt(self, node: Node):
        if isinstance(node, Block):
            saved = self.scope
            self.scope = Scope(self.scope)
            for s in node.stmts: self._gen_stmt(s)
            self.scope = saved
        elif isinstance(node, VarDecl):
            self._gen_local_var(node)
        elif isinstance(node, ExprStmt):
            self._gen_expr(node.expr)
        elif isinstance(node, ReturnStmt):
            if node.val: self._gen_expr(node.val)
            self._ins(f'jmp {self.func_ret_label}')
        elif isinstance(node, IfStmt):
            self._gen_if(node)
        elif isinstance(node, WhileStmt):
            self._gen_while(node)
        elif isinstance(node, DoWhile):
            self._gen_do_while(node)
        elif isinstance(node, ForStmt):
            self._gen_for(node)
        elif isinstance(node, BreakStmt):
            if self.break_labels: self._ins(f'jmp {self.break_labels[-1]}')
        elif isinstance(node, ContinueStmt):
            if self.continue_labels: self._ins(f'jmp {self.continue_labels[-1]}')
        elif isinstance(node, SwitchStmt):
            self._gen_switch(node)
        elif isinstance(node, CaseStmt):
            # Should be handled by switch
            self._gen_stmt(node.stmt)
        else:
            pass  # ignore

    def _gen_local_var(self, node: VarDecl):
        ty = node.ty
        sz = ty.size()
        if sz == 0: sz = 8
        al = ty.align()

        if node.is_static:
            # static local: becomes a global with a generated name
            lbl = f'.SL{self.static_cnt}_{node.name}'
            self.static_cnt += 1
            sym = Symbol(node.name, ty, True, label=lbl, is_static_local=True)
            self.scope.define(sym)
            # Emit in BSS or data later - for now emit inline
            old_out = self.out
            self.out = []
            self._emit(f'\t.local {lbl}')
            if node.init:
                self._emit(f'\t.data'); self._emit(f'{lbl}:')
                self._gen_global_init(ty, node.init)
            else:
                self._emit(f'\t.comm {lbl},{sz},{al}')
            static_decl = self.out
            self.out = old_out
            # Insert at beginning (after .file)
            self.out[1:1] = static_decl
            return

        # Align
        self.local_offset -= sz
        if al > 0: self.local_offset = (self.local_offset) & ~(al-1) if al > 1 else self.local_offset
        offset = self.local_offset

        sym = Symbol(node.name, ty, False, offset=offset)
        self.scope.define(sym)

        if node.init:
            if isinstance(node.init, BinOp) and node.init.op == '{}':
                # array/struct initializer
                items = node.init.right.stmts if isinstance(node.init.right, Block) else []
                if ty.kind == TypeKind.ARRAY and ty.base:
                    esz = ty.base.size()
                    for i, item in enumerate(items):
                        self._gen_expr(item)
                        item_off = offset + i * esz
                        self._store_size(esz, item_off)
                else:
                    for i, item in enumerate(items):
                        self._gen_expr(item)
                        self._store_size(8, offset + i*8)
            elif ty.kind == TypeKind.ARRAY and isinstance(node.init, StrLit):
                # char arr[] = "..."
                s = node.init.val + '\0'
                for i,ch in enumerate(s[:ty.array_size or len(s)]):
                    self._ins(f'movb ${ord(ch)}, {offset+i}(%rbp)')
            else:
                self._gen_expr(node.init)
                self._store_result(ty, offset)
        else:
            # zero-initialize
            if sz <= 8:
                self._ins(f'movq $0, {offset}(%rbp)')
            else:
                # zero out with rep stosq
                self._ins(f'leaq {offset}(%rbp), %rdi')
                self._ins(f'movq ${(sz+7)//8}, %rcx')
                self._ins(f'xorq %rax, %rax')
                self._ins(f'rep stosq')

    def _store_size(self, sz, offset):
        suf = self._size_suf(sz)
        reg = self._reg_for_size(sz)
        self._ins(f'mov{suf} {reg}, {offset}(%rbp)')

    def _store_result(self, ty: CType, offset: int):
        sz = ty.size()
        if sz == 0: return
        suf = self._size_suf(min(sz, 8))
        reg = self._reg_for_size(min(sz, 8))
        self._ins(f'mov{suf} {reg}, {offset}(%rbp)')

    # ── Control flow ───────────────────────────────────────────

    def _gen_if(self, node: IfStmt):
        lelse = self._new_label()
        lend = self._new_label()
        self._gen_expr(node.cond)
        self._ins('testq %rax, %rax')
        if node.els:
            self._ins(f'jz {lelse}')
            self._gen_stmt(node.then)
            self._ins(f'jmp {lend}')
            self._emit(f'{lelse}:')
            self._gen_stmt(node.els)
            self._emit(f'{lend}:')
        else:
            self._ins(f'jz {lend}')
            self._gen_stmt(node.then)
            self._emit(f'{lend}:')

    def _gen_while(self, node: WhileStmt):
        lcond = self._new_label()
        lend = self._new_label()
        self.break_labels.append(lend)
        self.continue_labels.append(lcond)
        self._emit(f'{lcond}:')
        self._gen_expr(node.cond)
        self._ins('testq %rax, %rax')
        self._ins(f'jz {lend}')
        self._gen_stmt(node.body)
        self._ins(f'jmp {lcond}')
        self._emit(f'{lend}:')
        self.break_labels.pop(); self.continue_labels.pop()

    def _gen_do_while(self, node: DoWhile):
        lbody = self._new_label()
        lcond = self._new_label()
        lend = self._new_label()
        self.break_labels.append(lend)
        self.continue_labels.append(lcond)
        self._emit(f'{lbody}:')
        self._gen_stmt(node.body)
        self._emit(f'{lcond}:')
        self._gen_expr(node.cond)
        self._ins('testq %rax, %rax')
        self._ins(f'jnz {lbody}')
        self._emit(f'{lend}:')
        self.break_labels.pop(); self.continue_labels.pop()

    def _gen_for(self, node: ForStmt):
        lcond = self._new_label()
        linc = self._new_label()
        lend = self._new_label()
        saved = self.scope; self.scope = Scope(self.scope)
        self.break_labels.append(lend)
        self.continue_labels.append(linc)
        if node.init: self._gen_stmt(node.init)
        self._emit(f'{lcond}:')
        if node.cond:
            self._gen_expr(node.cond)
            self._ins('testq %rax, %rax')
            self._ins(f'jz {lend}')
        self._gen_stmt(node.body)
        self._emit(f'{linc}:')
        if node.inc: self._gen_expr(node.inc)
        self._ins(f'jmp {lcond}')
        self._emit(f'{lend}:')
        self.break_labels.pop(); self.continue_labels.pop()
        self.scope = saved

    def _gen_switch(self, node: SwitchStmt):
        lend = self._new_label()
        self.break_labels.append(lend)
        # Evaluate switch expr
        self._gen_expr(node.expr)
        self._ins('movq %rax, %r12')  # save switch val

        # Collect cases from body
        cases = []
        ldefault = None
        self._collect_cases(node.body, cases)
        
        # Generate comparison jumps
        case_labels = {}
        for val, _ in cases:
            lbl = self._new_label()
            if val is None: ldefault = lbl
            else:
                case_labels[id(val)] = lbl
                if isinstance(val, IntLit):
                    self._ins(f'cmpq ${val.val}, %r12')
                    self._ins(f'je {lbl}')
        
        if ldefault: self._ins(f'jmp {ldefault}')
        else: self._ins(f'jmp {lend}')
        
        # Generate body with case labels
        self._gen_switch_body(node.body, cases, case_labels, ldefault)
        self._emit(f'{lend}:')
        self.break_labels.pop()

    def _collect_cases(self, node, cases):
        if isinstance(node, CaseStmt):
            cases.append((node.val, node))
            self._collect_cases(node.stmt, cases)
        elif isinstance(node, Block):
            for s in node.stmts: self._collect_cases(s, cases)

    def _gen_switch_body(self, node, cases, case_labels, ldefault):
        if isinstance(node, CaseStmt):
            if node.val is None and ldefault:
                self._emit(f'{ldefault}:')
            elif node.val is not None and id(node.val) in case_labels:
                self._emit(f'{case_labels[id(node.val)]}:')
            self._gen_stmt(node.stmt)
        elif isinstance(node, Block):
            for s in node.stmts:
                self._gen_switch_body(s, cases, case_labels, ldefault)
        else:
            self._gen_stmt(node)

    # ── Expression generation ─────────────────────────────────

    def _gen_expr(self, node: Node):
        """Generate expression, result in %rax"""
        if isinstance(node, IntLit):
            self._ins(f'movq ${node.val}, %rax')

        elif isinstance(node, StrLit):
            label = self._new_label('.LC')
            self.str_lits.append((label, node.val))
            self._ins(f'leaq {label}(%rip), %rax')

        elif isinstance(node, Var):
            self._gen_load_var(node)

        elif isinstance(node, BinOp):
            self._gen_binop(node)

        elif isinstance(node, UnaryOp):
            self._gen_unaryop(node)

        elif isinstance(node, Assign):
            self._gen_assign(node)

        elif isinstance(node, Call):
            self._gen_call(node)

        elif isinstance(node, Index):
            self._gen_index(node)

        elif isinstance(node, Member):
            self._gen_member(node)

        elif isinstance(node, Cast):
            self._gen_expr(node.expr)
            self._gen_cast(self._type_of(node.expr), node.ty)

        elif isinstance(node, Sizeof):
            if isinstance(node.arg, CType):
                sz = node.arg.size()
            else:
                ty = self._type_of(node.arg)
                sz = ty.size()
            self._ins(f'movq ${sz}, %rax')

        elif isinstance(node, Ternary):
            lelse = self._new_label(); lend = self._new_label()
            self._gen_expr(node.cond)
            self._ins('testq %rax, %rax')
            self._ins(f'jz {lelse}')
            self._gen_expr(node.then)
            self._ins(f'jmp {lend}')
            self._emit(f'{lelse}:')
            self._gen_expr(node.els)
            self._emit(f'{lend}:')

        elif isinstance(node, Comma):
            for e in node.exprs: self._gen_expr(e)

        else:
            self._ins('xorq %rax, %rax')  # fallback

    def _gen_load_var(self, node: Var):
        sym = self.scope.lookup(node.name)
        if sym is None:
            # Might be an enum constant
            if node.name in self.enum_vals:
                self._ins(f'movq ${self.enum_vals[node.name]}, %rax'); return
            # External symbol
            self._ins(f'movq {node.name}@GOTPCREL(%rip), %rax')
            self._ins(f'movq (%rax), %rax'); return

        if sym.is_global or sym.is_static_local:
            ty = sym.ty
            if ty.kind in (TypeKind.ARRAY, TypeKind.STRUCT):
                self._ins(f'leaq {sym.label}(%rip), %rax')
            else:
                sz = ty.size()
                if sz == 1:
                    if ty.is_signed(): self._ins(f'movsbq {sym.label}(%rip), %rax')
                    else: self._ins(f'movzbq {sym.label}(%rip), %rax')
                elif sz == 4:
                    if ty.is_signed(): self._ins(f'movslq {sym.label}(%rip), %rax')
                    else: self._ins(f'movl {sym.label}(%rip), %eax')
                else:
                    self._ins(f'movq {sym.label}(%rip), %rax')
        else:
            ty = sym.ty
            if ty.kind in (TypeKind.ARRAY, TypeKind.STRUCT):
                self._ins(f'leaq {sym.offset}(%rbp), %rax')
            else:
                sz = ty.size()
                off = sym.offset
                if sz == 1:
                    if ty.is_signed(): self._ins(f'movsbq {off}(%rbp), %rax')
                    else: self._ins(f'movzbq {off}(%rbp), %rax')
                elif sz == 4:
                    if ty.is_signed(): self._ins(f'movslq {off}(%rbp), %rax')
                    else: self._ins(f'movl {off}(%rbp), %eax')
                else:
                    self._ins(f'movq {off}(%rbp), %rax')

    def _gen_lvalue_addr(self, node: Node):
        """Generate address of lvalue into %rax"""
        if isinstance(node, Var):
            sym = self.scope.lookup(node.name)
            if sym is None:
                self._ins(f'leaq {node.name}(%rip), %rax'); return
            if sym.is_global or sym.is_static_local:
                self._ins(f'leaq {sym.label}(%rip), %rax')
            else:
                self._ins(f'leaq {sym.offset}(%rbp), %rax')
        elif isinstance(node, UnaryOp) and node.op == '*':
            self._gen_expr(node.expr)
        elif isinstance(node, Index):
            self._gen_index_addr(node)
        elif isinstance(node, Member):
            self._gen_member_addr(node)
        else:
            self._gen_expr(node)

    def _gen_index_addr(self, node: Index):
        arr_ty = self._type_of(node.arr)
        if arr_ty.kind == TypeKind.PTR:
            elem_ty = arr_ty.base
        elif arr_ty.kind == TypeKind.ARRAY:
            elem_ty = arr_ty.base
        else:
            elem_ty = T_INT
        esz = elem_ty.size() if elem_ty else 8
        # Get base address
        if arr_ty.kind == TypeKind.PTR:
            self._gen_expr(node.arr)
        else:
            self._gen_lvalue_addr(node.arr)
        self._ins('movq %rax, %r13')
        self._gen_expr(node.idx)
        self._ins(f'imulq ${esz}, %rax')
        self._ins('addq %r13, %rax')

    def _gen_index(self, node: Index):
        arr_ty = self._type_of(node.arr)
        if arr_ty.kind == TypeKind.PTR: elem_ty = arr_ty.base
        elif arr_ty.kind == TypeKind.ARRAY: elem_ty = arr_ty.base
        else: elem_ty = T_INT
        esz = elem_ty.size() if elem_ty else 8
        self._gen_index_addr(node)
        # Load from address
        self._gen_deref(elem_ty, '%rax')
        # Update type
        if hasattr(node, 'ty'): node.ty = elem_ty or T_INT

    def _gen_deref(self, ty: CType, reg='%rax'):
        sz = ty.size() if ty else 8
        if ty and ty.kind in (TypeKind.ARRAY, TypeKind.STRUCT):
            return  # address is value
        if sz == 1:
            if ty and ty.is_signed(): self._ins(f'movsbq ({reg}), %rax')
            else: self._ins(f'movzbq ({reg}), %rax')
        elif sz == 4:
            if ty and ty.is_signed(): self._ins(f'movslq ({reg}), %rax')
            else: self._ins(f'movl ({reg}), %eax')
        else:
            self._ins(f'movq ({reg}), %rax')

    def _gen_member_addr(self, node: Member):
        if node.is_ptr:
            self._gen_expr(node.expr)
        else:
            self._gen_lvalue_addr(node.expr)
        # Find field offset
        expr_ty = self._type_of(node.expr)
        if node.is_ptr and expr_ty.kind == TypeKind.PTR:
            struct_ty = expr_ty.base
        else:
            struct_ty = expr_ty
        if struct_ty and struct_ty.kind == TypeKind.STRUCT and struct_ty.members:
            for mname, mty, moff in struct_ty.members:
                if mname == node.field_name:
                    if moff != 0:
                        self._ins(f'addq ${moff}, %rax')
                    if hasattr(node, 'ty'): node.ty = mty
                    return

    def _gen_member(self, node: Member):
        expr_ty = self._type_of(node.expr)
        if node.is_ptr and expr_ty.kind == TypeKind.PTR:
            struct_ty = expr_ty.base
        else:
            struct_ty = expr_ty
        field_ty = T_INT
        if struct_ty and struct_ty.kind == TypeKind.STRUCT and struct_ty.members:
            for mname, mty, _ in struct_ty.members:
                if mname == node.field_name: field_ty = mty; break
        self._gen_member_addr(node)
        if hasattr(node, 'ty'): node.ty = field_ty
        if field_ty.kind not in (TypeKind.ARRAY, TypeKind.STRUCT):
            self._gen_deref(field_ty, '%rax')

    def _gen_binop(self, node: BinOp):
        op = node.op
        # Short-circuit operators
        if op == '&&':
            lfalse = self._new_label(); lend = self._new_label()
            self._gen_expr(node.left); self._ins('testq %rax,%rax'); self._ins(f'jz {lfalse}')
            self._gen_expr(node.right); self._ins('testq %rax,%rax'); self._ins(f'jz {lfalse}')
            self._ins('movq $1,%rax'); self._ins(f'jmp {lend}')
            self._emit(f'{lfalse}:'); self._ins('xorq %rax,%rax'); self._emit(f'{lend}:')
            return
        if op == '||':
            ltrue = self._new_label(); lend = self._new_label()
            self._gen_expr(node.left); self._ins('testq %rax,%rax'); self._ins(f'jnz {ltrue}')
            self._gen_expr(node.right); self._ins('testq %rax,%rax'); self._ins(f'jnz {ltrue}')
            self._ins('xorq %rax,%rax'); self._ins(f'jmp {lend}')
            self._emit(f'{ltrue}:'); self._ins('movq $1,%rax'); self._emit(f'{lend}:')
            return

        # Evaluate left, push; eval right; pop left
        lty = self._type_of(node.left)
        rty = self._type_of(node.right)
        self._gen_expr(node.left)
        self._ins('pushq %rax')
        self._gen_expr(node.right)
        self._ins('movq %rax, %rcx')   # rcx = right
        self._ins('popq %rax')         # rax = left

        # Pointer arithmetic adjustment
        if op in ('+','-') and lty.kind == TypeKind.PTR and rty.is_integer():
            esz = lty.base.size() if lty.base else 1
            if esz > 1: self._ins(f'imulq ${esz}, %rcx')
        elif op == '+' and rty.kind == TypeKind.PTR and lty.is_integer():
            esz = rty.base.size() if rty.base else 1
            if esz > 1: self._ins(f'imulq ${esz}, %rax')
            self._ins('xchgq %rax, %rcx')  # ptr in rax, offset in rcx

        if op == '+': self._ins('addq %rcx, %rax')
        elif op == '-':
            self._ins('subq %rcx, %rax')
            # Pointer subtraction -> divide by element size
            if lty.kind == TypeKind.PTR and rty.kind == TypeKind.PTR:
                esz = lty.base.size() if lty.base else 1
                if esz > 1:
                    self._ins(f'movq ${esz}, %rcx')
                    self._ins('cqto'); self._ins('idivq %rcx')
        elif op == '*': self._ins('imulq %rcx, %rax')
        elif op == '/':
            self._ins('cqto'); self._ins('idivq %rcx')
        elif op == '%':
            self._ins('cqto'); self._ins('idivq %rcx'); self._ins('movq %rdx, %rax')
        elif op == '&': self._ins('andq %rcx, %rax')
        elif op == '|': self._ins('orq %rcx, %rax')
        elif op == '^': self._ins('xorq %rcx, %rax')
        elif op == '<<': self._ins('movq %rcx, %rcx'); self._ins('salq %cl, %rax')
        elif op == '>>':
            if lty.is_signed(): self._ins('sarq %cl, %rax')
            else: self._ins('shrq %cl, %rax')
        elif op in ('==','!=','<','>','<=','>='):
            self._ins('cmpq %rcx, %rax')
            jmap = {'==':'sete','!=':'setne','<':'setl','>':'setg','<=':'setle','>=':'setge'}
            if lty.kind==TypeKind.PTR or not lty.is_signed():
                jmap.update({'<':'setb','>':'seta','<=':'setbe','>=':'setae'})
            self._ins(f'{jmap[op]} %al')
            self._ins('movzbq %al, %rax')

    def _gen_unaryop(self, node: UnaryOp):
        op = node.op
        ty = self._type_of(node.expr)

        if op == '-':
            self._gen_expr(node.expr); self._ins('negq %rax')
        elif op == '~':
            self._gen_expr(node.expr); self._ins('notq %rax')
        elif op == '!':
            self._gen_expr(node.expr); self._ins('testq %rax,%rax')
            self._ins('sete %al'); self._ins('movzbq %al, %rax')
        elif op == '&':
            self._gen_lvalue_addr(node.expr)
        elif op == '*':
            self._gen_expr(node.expr)
            elem_ty = ty.base if ty.kind == TypeKind.PTR else T_INT
            self._gen_deref(elem_ty, '%rax')
            if hasattr(node, 'ty'): node.ty = elem_ty or T_INT
        elif op == 'pre++':
            self._gen_lvalue_addr(node.expr)
            self._ins('movq %rax, %r13')
            esz = ty.base.size() if ty.kind==TypeKind.PTR else 1
            sz = ty.size()
            if sz==1: self._ins(f'addb ${esz}, (%r13)')
            elif sz==4: self._ins(f'addl ${esz}, (%r13)')
            else: self._ins(f'addq ${esz}, (%r13)')
            self._gen_deref(ty, '%r13'); self._ins('movq %rax, %r13')  # re-read
            # Actually just load new value
            if sz==1: self._ins('movsbq (%r13),%rax')
            elif sz==4: self._ins('movslq (%r13),%rax')
            else: self._ins('movq (%r13),%rax')
        elif op == 'pre--':
            self._gen_lvalue_addr(node.expr)
            self._ins('movq %rax, %r13')
            esz = ty.base.size() if ty.kind==TypeKind.PTR else 1
            sz = ty.size()
            if sz==1: self._ins(f'subb ${esz}, (%r13)')
            elif sz==4: self._ins(f'subl ${esz}, (%r13)')
            else: self._ins(f'subq ${esz}, (%r13)')
            if sz==1: self._ins('movsbq (%r13),%rax')
            elif sz==4: self._ins('movslq (%r13),%rax')
            else: self._ins('movq (%r13),%rax')
        elif op == 'post++':
            self._gen_lvalue_addr(node.expr)
            self._ins('movq %rax, %r13')
            sz = ty.size(); esz = ty.base.size() if ty.kind==TypeKind.PTR else 1
            if sz==1: self._ins('movsbq (%r13),%rax')
            elif sz==4: self._ins('movslq (%r13),%rax')
            else: self._ins('movq (%r13),%rax')
            self._ins('movq %rax, %r14')
            if sz==1: self._ins(f'addb ${esz},(%r13)')
            elif sz==4: self._ins(f'addl ${esz},(%r13)')
            else: self._ins(f'addq ${esz},(%r13)')
            self._ins('movq %r14, %rax')
        elif op == 'post--':
            self._gen_lvalue_addr(node.expr)
            self._ins('movq %rax, %r13')
            sz = ty.size(); esz = ty.base.size() if ty.kind==TypeKind.PTR else 1
            if sz==1: self._ins('movsbq (%r13),%rax')
            elif sz==4: self._ins('movslq (%r13),%rax')
            else: self._ins('movq (%r13),%rax')
            self._ins('movq %rax, %r14')
            if sz==1: self._ins(f'subb ${esz},(%r13)')
            elif sz==4: self._ins(f'subl ${esz},(%r13)')
            else: self._ins(f'subq ${esz},(%r13)')
            self._ins('movq %r14, %rax')

    def _gen_assign(self, node: Assign):
        op = node.op
        ty = self._type_of(node.target)

        if op != '=':
            # compound: evaluate lhs old value
            simple_op = op[:-1]  # += -> +, etc.
            self._gen_lvalue_addr(node.target)
            self._ins('movq %rax, %r13')
            # load old value
            self._gen_deref(ty, '%r13')
            self._ins('pushq %rax')
            # evaluate rhs
            self._gen_expr(node.value)
            self._ins('movq %rax, %rcx')
            self._ins('popq %rax')
            # apply op
            if simple_op == '+':
                if ty.kind == TypeKind.PTR:
                    esz = ty.base.size() if ty.base else 1
                    if esz>1: self._ins(f'imulq ${esz},%rcx')
                self._ins('addq %rcx,%rax')
            elif simple_op == '-':
                if ty.kind == TypeKind.PTR:
                    esz = ty.base.size() if ty.base else 1
                    if esz>1: self._ins(f'imulq ${esz},%rcx')
                self._ins('subq %rcx,%rax')
            elif simple_op == '*': self._ins('imulq %rcx,%rax')
            elif simple_op == '/': self._ins('cqto'); self._ins('idivq %rcx')
            elif simple_op == '%': self._ins('cqto'); self._ins('idivq %rcx'); self._ins('movq %rdx,%rax')
            elif simple_op == '&': self._ins('andq %rcx,%rax')
            elif simple_op == '|': self._ins('orq %rcx,%rax')
            elif simple_op == '^': self._ins('xorq %rcx,%rax')
            elif simple_op == '<<': self._ins('salq %cl,%rax')
            elif simple_op == '>>': self._ins('sarq %cl,%rax')
            # store back
            self._gen_store_through(ty, '%r13', '%rax')
            return

        # Simple assignment
        self._gen_expr(node.value)
        self._ins('movq %rax, %r14')  # save value
        self._gen_lvalue_addr(node.target)
        self._gen_store_through(ty, '%rax', '%r14')
        self._ins('movq %r14, %rax')

    def _gen_store_through(self, ty: CType, addr_reg: str, val_reg: str):
        """Store val_reg through addr_reg"""
        sz = ty.size()
        if sz == 0: return
        if ty.kind in (TypeKind.ARRAY, TypeKind.STRUCT):
            # struct copy
            self._ins(f'movq {val_reg}, %rsi')
            self._ins(f'movq {addr_reg}, %rdi')
            self._ins(f'movq ${(sz+7)//8}, %rcx')
            self._ins('rep movsq')
            return
        val = self._reg_for_size(min(sz,8), val_reg)
        suf = self._size_suf(min(sz,8))
        self._ins(f'mov{suf} {val}, ({addr_reg})')

    def _gen_call(self, node: Call):
        # Determine function name for direct call
        func_name = None
        if isinstance(node.func, Var): func_name = node.func.name

        args = node.args
        nargs = len(args)

        # Evaluate args and push onto stack (in reverse for stack args)
        # For register args, we need to evaluate in order and temporarily store
        # Strategy: evaluate all args, push on stack, then load into regs
        
        # Align stack: after pushq rbp, 4 callee regs = 5 pushes = 40 bytes off
        # We need stack aligned to 16 at call site
        # Current rsp is: original - 8(rbp) - 8*4(callee) - frame_size
        # frame_size is already 16-aligned, 40 extra pushes -> rsp % 16 = 8
        # So we need an odd number of extra pushes for 16-byte alignment
        
        extra_stack_args = max(0, nargs - 6)
        # Pad to align stack
        total_push = extra_stack_args
        if (total_push % 2) != 0:
            self._ins('subq $8, %rsp')  # alignment pad
            total_push += 1

        # Evaluate args in order, store temporarily
        arg_offsets = []
        for i, arg in enumerate(args):
            self._gen_expr(arg)
            self._ins('pushq %rax')
            arg_offsets.append(i)

        # Now load register args (first 6) from stack
        # Stack layout: args pushed in order, top = last arg
        # args[0] is at rsp + (nargs-1)*8
        for i in range(min(nargs, 6)-1, -1, -1):
            stack_pos = (nargs-1-i)*8
            reg = PARAM_REGS_64[i]
            self._ins(f'movq {stack_pos}(%rsp), {reg}')

        # Move register args off stack
        if nargs <= 6:
            self._ins(f'addq ${nargs*8}, %rsp')
        else:
            # For stack args (>6), leave on stack in correct order
            # Currently stack = [arg0, arg1, ..., argN-1], top=argN-1
            # SysV wants stack args at rsp+0=arg6, rsp+8=arg7 at call time
            # We have arg6..argN-1 below the reg args
            # Pop register args
            self._ins(f'addq ${6*8}, %rsp')
            # Stack args are now at rsp = arg6..argN-1 (arg6 at top)
            # But we need arg6 at lower address... they're already in right order (arg6 at rsp)

        # Zero %al for variadic
        self._ins('xorb %al, %al')

        if func_name:
            self._ins(f'callq {func_name}@PLT')
        else:
            self._gen_expr(node.func)
            self._ins('callq *%rax')

        # Clean up stack args
        if nargs > 6:
            self._ins(f'addq ${extra_stack_args*8}, %rsp')
        # Remove alignment pad
        if (max(0,nargs-6) % 2) != 0:
            self._ins('addq $8, %rsp')

        # Set return type
        if func_name and func_name in self.func_sigs:
            ret_ty, _, _ = self.func_sigs[func_name]
            if hasattr(node, 'ty'): node.ty = ret_ty

        # Sign/zero extend if needed
        # For now, rax holds return value

    def _gen_cast(self, from_ty: CType, to_ty: CType):
        if from_ty.kind == to_ty.kind: return
        fsz = from_ty.size(); tsz = to_ty.size()
        if to_ty.kind == TypeKind.VOID: return
        if fsz == tsz: return
        if tsz > fsz:
            # Sign/zero extend
            if from_ty.is_signed():
                ext = {(1,4):'movsbq',(1,8):'movsbq',(2,4):'movswq',(2,8):'movswq',(4,8):'movslq'}.get((fsz,tsz))
                if ext: self._ins(f'{ext} %al, %rax')
            # else zero extend (movzx)
        elif tsz < fsz:
            # Truncate (already in lower bytes of rax)
            pass

# ─────────────────────────────────────────────────────────────
# §9  COMPILER DRIVER
# ─────────────────────────────────────────────────────────────
class Compiler:
    def __init__(self, verbose=False):
        self.verbose = verbose

    def preprocess(self, src: str, filename: str) -> str:
        pp = Preprocessor(filename)
        return pp.process(src, filename)

    def compile_to_asm(self, src: str, filename: str) -> str:
        if self.verbose: print(f"[lex] {filename}", file=sys.stderr)
        lexer = Lexer(src, filename)
        if self.verbose: print(f"[parse] {len(lexer.tokens)} tokens", file=sys.stderr)
        parser = Parser(lexer.tokens, filename)
        tu = parser.parse()
        if self.verbose: print(f"[codegen] {len(tu.decls)} decls", file=sys.stderr)
        cg = CodeGen(filename)
        cg.global_scope = parser.global_scope if hasattr(parser,'global_scope') else cg.global_scope
        # Pre-register typedef names
        for k,v in parser.typedefs.items():
            pass  # codegen doesn't need them for generation
        # Pre-register struct info
        cg.structs = parser.structs
        cg.generate(tu)
        return cg.get_assembly()

    def compile_file(self, input_path: str, output_path: str, mode='exe'):
        src = open(input_path).read()
        filename = os.path.basename(input_path)

        # Preprocess
        pp = Preprocessor(input_path)
        processed = pp.process(src, input_path)

        # Compile to assembly
        try:
            asm = self.compile_to_asm(processed, filename)
        except SyntaxError as e:
            print(f"Error: {e}", file=sys.stderr); sys.exit(1)
        except Exception as e:
            print(f"Internal error: {e}", file=sys.stderr)
            if self.verbose: import traceback; traceback.print_exc()
            sys.exit(1)

        if mode == 'S':
            out = output_path or os.path.splitext(input_path)[0]+'.s'
            open(out,'w').write(asm); return out

        # Assemble
        asm_file = tempfile.NamedTemporaryFile(suffix='.s', delete=False, mode='w')
        asm_file.write(asm); asm_file.close()

        if mode == 'c':
            obj_out = output_path or os.path.splitext(input_path)[0]+'.o'
            ret = subprocess.run(['gcc','-c',asm_file.name,'-o',obj_out])
            os.unlink(asm_file.name)
            if ret.returncode != 0: sys.exit(ret.returncode)
            return obj_out

        # mode == 'exe': compile + link
        exe_out = output_path or os.path.splitext(input_path)[0]
        ret = subprocess.run(['gcc', asm_file.name, '-o', exe_out, '-lm'])
        os.unlink(asm_file.name)
        if ret.returncode != 0: sys.exit(ret.returncode)
        return exe_out

def main():
    import argparse
    ap = argparse.ArgumentParser(description='ZKAEDI-CC: Self-Hosting C Compiler (Stage 0)')
    ap.add_argument('input', help='Input .c file')
    ap.add_argument('-o','--output', help='Output file')
    ap.add_argument('-S','--asm', action='store_true', help='Emit assembly')
    ap.add_argument('-c','--obj', action='store_true', help='Emit object file')
    ap.add_argument('-E','--preprocess', action='store_true', help='Preprocess only')
    ap.add_argument('-v','--verbose', action='store_true')
    ap.add_argument('-I', action='append', default=[], dest='includes', help='Include path')
    args = ap.parse_args()

    cc = Compiler(verbose=args.verbose)

    if args.preprocess:
        src = open(args.input).read()
        pp = Preprocessor(args.input)
        print(pp.process(src, args.input)); return

    mode = 'exe'
    if args.asm: mode = 'S'
    elif args.obj: mode = 'c'

    out = cc.compile_file(args.input, args.output, mode=mode)
    if args.verbose: print(f"[done] {out}", file=sys.stderr)

if __name__ == '__main__':
    main()
