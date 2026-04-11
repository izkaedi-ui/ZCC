import subprocess, os
env = os.environ.copy()
env['ZCC_EMIT_IR'] = '1'
print('Starting...')
r = subprocess.run(['./zcc2', 'sqlite3_zcc.c'], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
print('RC:', r.returncode)
print('ERR:', r.stderr[:200])
