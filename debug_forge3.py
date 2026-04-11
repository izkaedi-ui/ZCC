import subprocess, os
env = os.environ.copy()
env['ZCC_EMIT_IR'] = '1'
env['ZCC_IR_BACKEND'] = '1'
p = subprocess.Popen(['./zcc2', 'sqlite3_zcc.c', '-o', 'tmp.s'], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p.communicate()
print('RC=', p.returncode)
