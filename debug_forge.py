import subprocess, os
env = os.environ.copy()
env['ZCC_EMIT_IR'] = '1'
p = subprocess.Popen(['./zcc2', 'sqlite3_zcc.c'], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p.communicate()
print('RC=', p.returncode)
print('OUT len:', len(out))
print('ERR len:', len(err))
with open('debug_out.txt', 'wb') as f: f.write(out[:1000])
with open('debug_err.txt', 'wb') as f: f.write(err[:1000])
