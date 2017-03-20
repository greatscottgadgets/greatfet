import sys

with open(sys.argv[2], 'wb') as outfile:
    with open(sys.argv[1], 'rb') as infile:
        print "[+] copying %s to %s with ensmallification" % (sys.argv[1], sys.argv[2])
        outfile.write(infile.read(0x00002400))
