#!/usr/bin/env node
/**
 * EVM HEALER — MAINNET VOLUME EXTRACTION
 * =======================================
 * Fetches top verified Solidity contracts from Etherscan,
 * extracts 64-D tensors, and appends to training_corpus.json.
 *
 * Usage:
 *   ETHERSCAN_API_KEY=YourKey npx ts-node tools/etherscan_batch_fetch.ts
 *   ETHERSCAN_API_KEY=YourKey npx ts-node tools/etherscan_batch_fetch.ts --limit 200 --delay 220
 */

import * as fs from 'fs';
import * as path from 'path';
import * as https from 'https';
import parser from '@solidity-parser/parser';

// ============================================
// CONFIG
// ============================================
import * as dotenv from 'dotenv';
dotenv.config();

const API_KEY = process.env.ETHERSCAN_API_KEY;
if (!API_KEY) {
    console.error('[!] ETHERSCAN_API_KEY required. Get one at https://etherscan.io/myapikey');
    process.exit(1);
}

const ARGS = parseArgs();
const DELAY_MS = ARGS.delay || 220; // ~4.5 req/sec (free tier: 5/sec)
const FETCH_LIMIT = ARGS.limit || 200;
const CORPUS_PATH = ARGS.corpus || 'training_corpus.json';
const OUTPUT_DIR = ARGS.outdir || 'mainnet_contracts';
const CHAIN_ID = ARGS.chain || 1;

// ============================================
// KNOWN HIGH-VALUE TARGETS
// ============================================

// Top Ethereum contracts by historical transaction volume / DeFi TVL
// Curated for maximal tensor diversity
const HIGH_VALUE_TARGETS: Array<{ address: string; name: string; note: string }> = [
    // DEX Routers — dense call graphs, multi-external-call
    { address: '0x7a250d5630B4cF539739dF2C5dAcb4c659F2488D', name: 'UniswapV2Router02', note: 'DEX router, dense calls' },
    { address: '0xE592427A0AEce92De3Edee1F18E0157C05861564', name: 'UniswapV3Router', note: 'V3 swap router' },
    { address: '0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F', name: 'SushiSwapRouter', note: 'Sushi DEX router' },
    { address: '0x1111111254EEB25477B68fb85Ed929f73A960582', name: '1inchV5Router', note: 'Aggregator, complex routing' },

    // Lending — proxy patterns, delegatecall, complex state
    { address: '0x7d2768dE32b0b80b7a3454c06BdAc94A69DDc7A9', name: 'AaveLendingPoolV2', note: 'Lending pool, proxy' },
    { address: '0x3d9819210A31b4961b30EF54bE2aeD79B9c9Cd3B', name: 'CompoundComptroller', note: 'Comptroller, access ctrl' },
    { address: '0x4Ddc2D193948926D02f9B1fE9e1daa0718270ED5', name: 'CompoundcETH', note: 'cToken, delegatecall' },
    { address: '0x5d3a536E4D6DbD6114cc1Ead35777bAB948E3643', name: 'CompoundcDAI', note: 'cToken variant' },

    // Stablecoins / Tokens — pure token logic, should be SECURE baseline
    { address: '0xdAC17F958D2ee523a2206206994597C13D831ec7', name: 'TetherUSDT', note: 'ERC20 token, secure baseline' },
    { address: '0xA0b86991c6218b36c1d19D4a2e9Eb0cE3606eB48', name: 'USDC', note: 'ERC20 proxy token' },
    { address: '0x6B175474E89094C44Da98b954EedeAC495271d0F', name: 'DAI', note: 'MakerDAO stablecoin' },
    { address: '0xC02aaA39b223FE8D0A0e5C4F27eAD9083C756Cc2', name: 'WETH9', note: 'Wrapped ETH, minimal' },
    { address: '0x2260FAC5E5542a773Aa44fBCfeDf7C193bc2C599', name: 'WBTC', note: 'Wrapped BTC ERC20' },
    { address: '0x95aD61b0a150d79219dCF64E1E6Cc01f0B64C4cE', name: 'SHIB', note: 'Meme token, simple ERC20' },

    // Bridges / Multisig — high-risk patterns
    { address: '0x40ec5B33f54e0E8A33A975908C5BA1c14e5BbbDf', name: 'PolygonBridge', note: 'Bridge, complex state' },
    { address: '0x8EB8a3b98659Cce290402893d0123abb75E3ab28', name: 'GnosisMultisig', note: 'Multisig wallet' },

    // NFT — ERC721, callback patterns
    { address: '0xBC4CA0EdA7647A8aB7C2061c2E118A18a936f13D', name: 'BAYC', note: 'ERC721 NFT' },
    { address: '0x60E4d786628Fea6478F785A6d7e704777c86a7c6', name: 'MAYC', note: 'ERC721 NFT variant' },

    // Governance / Staking
    { address: '0xC0AEe478e3658e2610c5F7A4A2E1777cE9e4f2Ac', name: 'SushiFactory', note: 'Factory pattern' },
    { address: '0x5e4be8Bc9637f0EAA1A755019e06A68ce081D58F', name: 'SushiMasterChef', note: 'Staking, reward math' },

    // Exploited contracts — ground truth VULNERABLE
    { address: '0x863DF6BFa4469f3ead0bE8f9F2AAE51c91A907b4', name: 'ParityWalletLib', note: 'EXPLOITED: selfdestruct' },

    // Yield / Vaults
    { address: '0x9D25057e62939D3408406975aD75Ffe834DA4cDd', name: 'YearnRegistry', note: 'Vault registry' },

    // Oracle / Price feeds
    { address: '0x5f4eC3Df9cbd43714FE2740f5E3616155c5b8419', name: 'ChainlinkETHUSD', note: 'Oracle, view-only' },
];

// ============================================
// HTTP HELPERS
// ============================================

function sleep(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function httpsGet(url: string): Promise<string> {
    return new Promise((resolve, reject) => {
        https.get(url, (res) => {
            let data = '';
            res.on('data', (chunk) => data += chunk);
            res.on('end', () => resolve(data));
            res.on('error', reject);
        }).on('error', reject);
    });
}

interface FetchResult {
    name: string;
    source: string;
    compiler: string;
    address: string;
}

async function fetchSource(address: string): Promise<FetchResult | null> {
    const url = `https://api.etherscan.io/v2/api?chainid=${CHAIN_ID}&module=contract&action=getsourcecode&address=${address}&apikey=${API_KEY}`;

    try {
        const raw = await httpsGet(url);
        const data = JSON.parse(raw);

        if (data.status !== '1' || !data.result?.[0]) {
            return null;
        }

        const contract = data.result[0];
        let source = contract.SourceCode || '';

        if (!source || source === '') return null;

        // Unwrap JSON-encoded multi-file sources
        if (source.startsWith('{{')) {
            try {
                const multi = JSON.parse(source.slice(1, -1));
                const sources = multi.sources || {};
                source = Object.values(sources)
                    .map((s: any) => s.content)
                    .join('\n\n');
            } catch {
                try {
                    const single = JSON.parse(source);
                    source = Object.values(single.sources || {})
                        .map((s: any) => s.content)
                        .join('\n\n');
                } catch { /* use raw */ }
            }
        }

        return {
            name: contract.ContractName || 'Unknown',
            source,
            compiler: contract.CompilerVersion || '',
            address,
        };
    } catch (e: any) {
        return null;
    }
}

// ============================================
// TENSOR EXTRACTION (mirrors tensorTestRunner.ts)
// ============================================

function extractTensor(source: string): { tensor: number[]; error?: string } {
    const features = new Float32Array(64).fill(0.0);
    const stateVars = new Set<string>();
    let externalCallFired = false;

    let ast: any;
    try {
        ast = (parser as any).parse(source, { loc: true, tolerant: true } as any);
    } catch (e: any) {
        return { tensor: Array.from(features), error: e.message };
    }

    (parser as any).visit(ast, {
        ContractDefinition(node: any) {
            features[0]++;
            if (node.kind === 'library') features[1]++;
            if (node.kind === 'interface') features[2]++;
            if (node.baseContracts?.length > 0) features[3] += node.baseContracts.length;
        },
        UsingForDeclaration() { features[4]++; },
        StateVariableDeclaration(node: any) {
            features[10]++;
            node.variables?.forEach((v: any) => {
                if (v.name) stateVars.add(v.name);
                if (v.isDeclaredConst) features[12]++;
                if (v.typeName?.type === 'Mapping') features[14]++;
            });
        },
        EventDefinition() { features[7]++; },
        EmitStatement() { features[8]++; },
        EnumDefinition() { features[9]++; },
        StructDefinition() { features[9]++; },
        FunctionDefinition(node: any) {
            features[5]++;
            externalCallFired = false;
            if (node.stateMutability === 'payable') features[20]++;
            if (node.isConstructor) features[6]++;
            if (node.isFallback || node.isReceiveEther) features[23]++;
            if (node.modifiers?.length > 0) {
                features[30] += node.modifiers.length;
                // Detect nonReentrant guard (dim 19)
                for (const mod of node.modifiers) {
                    const modName = mod.name || mod.arguments?.[0]?.name || '';
                    if (/nonreentrant|reentrancyguard|noreentr/i.test(modName)) {
                        features[19]++;
                    }
                }
            }
        },
        ModifierDefinition(node: any) {
            features[31]++;
            const modName = node.name || '';
            if (/nonreentrant|reentrancyguard|noreentr/i.test(modName)) {
                features[19]++;
            }
        },
        AssemblyCall(node: any) {
            const name = node.functionName || node.name;
            if (name === 'delegatecall') { externalCallFired = true; features[21]++; features[60]++; }
            if (name === 'staticcall') { features[21]++; }
            if (name === 'create' || name === 'create2') { features[62]++; }
            if (name === 'selfdestruct') { features[63]++; }
            if (name === 'call') { externalCallFired = true; features[21]++; features[22]++; }
        },
        FunctionCall(node: any) {
            features[40]++;
            let callExpr = node.expression;
            if (callExpr.type === 'NameValueExpression') callExpr = callExpr.expression;
            if (callExpr.type === 'MemberAccess') {
                const member = callExpr.memberName;
                if (member === 'call') { externalCallFired = true; features[21]++; features[22]++; }
                else if (member === 'delegatecall') { externalCallFired = true; features[21]++; features[60]++; }
                else if (member === 'transfer' || member === 'send') { externalCallFired = true; features[21]++; features[24]++; }
            } else if (node.expression.type === 'Identifier') {
                const name = node.expression.name;
                if (name === 'require') features[32]++;
                if (name === 'assert') features[33]++;
                if (name === 'revert') features[34]++;
                if (name === 'selfdestruct' || name === 'suicide') features[63]++;
            }
        },
        AssemblyBlock() { features[61]++; },
        BinaryOperation(node: any) {
            const ASSIGN_OPS = new Set(['=', '+=', '-=', '*=', '/=', '%=', '|=', '&=', '^=', '<<=', '>>=']);
            if (ASSIGN_OPS.has(node.operator)) {
                features[13]++;
                if (node.left?.type === 'Identifier' && stateVars.has(node.left.name)) {
                    features[11]++;
                    if (externalCallFired) features[15]++;
                }
                if (node.left?.type === 'IndexAccess') {
                    const base = node.left.base;
                    if (base?.type === 'Identifier' && stateVars.has(base.name)) {
                        features[11]++;
                        if (externalCallFired) features[15]++;
                    }
                }
            } else {
                features[41]++;
                if (node.operator === '/' || node.operator === '%') features[42]++;
            }
        },
        UnaryOperation() { features[43]++; },
        Identifier(node: any) {
            if (node.name === 'msg' || node.name === 'tx' || node.name === 'block') features[35]++;
        },
        IfStatement() { features[50]++; },
        ForStatement() { features[51]++; },
        WhileStatement() { features[52]++; },
    } as unknown as any);

    // Density normalization
    const fnCount = Math.max(1, features[5]);
    const normalized = Array.from(features).map((val, i) => {
        if (i === 0 || i === 5) return val;
        return parseFloat((val / fnCount).toFixed(4));
    });

    return { tensor: normalized };
}

// ============================================
// BATCH PIPELINE
// ============================================

interface CorpusEntry {
    name: string;
    address: string;
    compiler: string;
    tensor: number[];
    label: string;
    source: string;
    note: string;
    fetched_at: string;
    parse_error?: string;
}

async function batchFetch() {
    console.log(`\n${'═'.repeat(60)}`);
    console.log('  EVM HEALER — MAINNET VOLUME EXTRACTION');
    console.log(`${'═'.repeat(60)}`);
    console.log(`  Targets    : ${HIGH_VALUE_TARGETS.length} curated + discovery`);
    console.log(`  Delay      : ${DELAY_MS}ms between requests`);
    console.log(`  Chain      : ${CHAIN_ID}`);
    console.log(`  Output     : ${CORPUS_PATH}`);
    console.log(`${'═'.repeat(60)}\n`);

    fs.mkdirSync(OUTPUT_DIR, { recursive: true });

    // Load existing corpus
    let corpus: CorpusEntry[] = [];
    if (fs.existsSync(CORPUS_PATH)) {
        try {
            const raw = JSON.parse(fs.readFileSync(CORPUS_PATH, 'utf8'));
            corpus = Array.isArray(raw) ? raw : (raw.entries || raw.data || raw.corpus || []);
            console.log(`  Loaded existing corpus: ${corpus.length} entries`);
        } catch { /* start fresh */ }
    }

    const existingAddresses = new Set(corpus.map(e => e.address?.toLowerCase()).filter(Boolean));

    let fetched = 0;
    let parsed = 0;
    let failed = 0;
    let skipped = 0;

    for (const target of HIGH_VALUE_TARGETS) {
        if (fetched >= FETCH_LIMIT) break;
        if (existingAddresses.has(target.address.toLowerCase())) {
            console.log(`  [SKIP] ${target.name} — already in corpus`);
            skipped++;
            continue;
        }

        process.stdout.write(`  [${fetched + 1}/${HIGH_VALUE_TARGETS.length}] ${target.name}...`);

        const result = await fetchSource(target.address);
        await sleep(DELAY_MS);
        fetched++;

        if (!result || !result.source) {
            console.log(' FAILED (no source)');
            failed++;
            continue;
        }

        // Save raw source
        const safeName = result.name.replace(/[^a-zA-Z0-9_-]/g, '_');
        const solPath = path.join(OUTPUT_DIR, `${safeName}_${target.address.slice(0, 8)}.sol`);
        fs.writeFileSync(solPath, result.source);

        // Extract tensor
        const { tensor, error } = extractTensor(result.source);

        const entry: CorpusEntry = {
            name: result.name,
            address: target.address,
            compiler: result.compiler,
            tensor,
            label: 'UNKNOWN', // Will be labeled during adversarial review
            source: result.source,
            note: target.note,
            fetched_at: new Date().toISOString(),
        };

        if (error) {
            entry.parse_error = error;
            console.log(` PARSED (tolerant, ${result.source.length} chars)`);
        } else {
            console.log(` OK (${result.source.length} chars, ${tensor.filter(v => v > 0).length}/64 dims active)`);
        }

        corpus.push(entry);
        parsed++;

        // Save corpus incrementally (crash-safe)
        fs.writeFileSync(CORPUS_PATH, JSON.stringify(corpus, null, 2));
    }

    // ============================================
    // SUMMARY
    // ============================================

    console.log(`\n${'═'.repeat(60)}`);
    console.log('  EXTRACTION COMPLETE');
    console.log(`${'═'.repeat(60)}`);
    console.log(`  Fetched    : ${fetched}`);
    console.log(`  Parsed     : ${parsed}`);
    console.log(`  Failed     : ${failed}`);
    console.log(`  Skipped    : ${skipped}`);
    console.log(`  Corpus     : ${corpus.length} total entries`);
    console.log(`  Saved to   : ${CORPUS_PATH}`);

    // Tensor density stats
    const mainnetEntries = corpus.filter(e => e.source === 'etherscan_mainnet');
    if (mainnetEntries.length > 0) {
        const densities = mainnetEntries.map(e => e.tensor.filter(v => v > 0).length);
        const avgDensity = densities.reduce((a, b) => a + b, 0) / densities.length;
        const criticalHits = mainnetEntries.filter(e =>
            e.tensor[15] > 0 || e.tensor[60] > 0 || e.tensor[63] > 0
        ).length;

        console.log(`\n  Mainnet Stats:`);
        console.log(`    Avg tensor density : ${avgDensity.toFixed(1)}/64 dims`);
        console.log(`    Critical dim hits  : ${criticalHits}/${mainnetEntries.length} contracts`);
        console.log(`    Contracts saved to : ${OUTPUT_DIR}/`);
    }

    console.log(`\n  Next: Label UNKNOWN entries, run adversarial traps, retrain.`);
    console.log(`  python train_adam_heuristics.py --corpus ${CORPUS_PATH} --epochs 500\n`);
}

// ============================================
// ARG PARSER
// ============================================

function parseArgs(): Record<string, any> {
    const args: Record<string, any> = {};
    const argv = process.argv.slice(2);
    for (let i = 0; i < argv.length; i++) {
        if (argv[i] === '--limit') args.limit = parseInt(argv[++i]);
        if (argv[i] === '--delay') args.delay = parseInt(argv[++i]);
        if (argv[i] === '--corpus') args.corpus = argv[++i];
        if (argv[i] === '--outdir') args.outdir = argv[++i];
        if (argv[i] === '--chain') args.chain = parseInt(argv[++i]);
    }
    return args;
}

batchFetch().catch(console.error);
