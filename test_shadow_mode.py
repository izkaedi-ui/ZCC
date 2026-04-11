import asyncio
import logging

logging.basicConfig(level=logging.INFO)

from zkaedi_supervisor_mev import build_supervisor

async def main():
    sup = build_supervisor()
    await sup.core.autonomous_synthesis_loop('0x123', sup.lora, sup.executor)

asyncio.run(main())
