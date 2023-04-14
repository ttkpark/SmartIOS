fin = open("smartios.bin","rb")
data = bytearray(fin.read())
fin.close()

for i in range(0,16) :
    (data[0x100 + i],data[0x11F - i]) = (data[0x11F - i],data[0x100 + i])

fout = open("smartios.pgm","wb")
fout.write(data)
fout.close()