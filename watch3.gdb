set pagination off
b sqlite3ParserInit
run
commands 1
  watch *(void**)()
  continue
end
continue
