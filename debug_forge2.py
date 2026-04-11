import subprocess, os
env = os.environ.copy()
env['ZCC_EMIT_IR'] = '1'
p = subprocess.Popen(['./zcc2', 'sqlite3_zcc.c'], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p.communicate()
with open('debug_tail.txt', 'wb') as f:
    f.write(out[-2000:])
