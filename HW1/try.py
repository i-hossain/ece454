import sys

orig_stdout = sys.stdout
f = file('out.txt', 'w')
sys.stdout = f

make clean

sys.stdout = orig_stdout
f.close()