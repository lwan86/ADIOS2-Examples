import sys

input_fn = sys.argv[1]
input_fd = open(input_fn, 'r')
for line in input_fd:
    #print line.split(' ')
    if line.split(' ')[0] == 'I/O':
        print line.split(' ')[3].strip()
input_fd.close()
