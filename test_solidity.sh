#!/bin/bash
curl -s -X POST http://localhost:8080/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model":"zkaedi-mini-q4_k_m.gguf",
    "messages":[
      {"role":"system","content":"You are ZKAEDI-MINI, a Solidity smart contract security auditor. Identify vulnerabilities and rate severity."},
      {"role":"user","content":"Audit this contract:\n\npragma solidity ^0.8.0;\n\ncontract Vault {\n    mapping(address => uint256) public balances;\n\n    function deposit() external payable {\n        balances[msg.sender] += msg.value;\n    }\n\n    function withdraw() external {\n        uint256 bal = balances[msg.sender];\n        (bool sent, ) = msg.sender.call{value: bal}(\"\");\n        require(sent, \"Failed\");\n        balances[msg.sender] = 0;\n    }\n}"}
    ],
    "max_tokens":500
  }' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"
