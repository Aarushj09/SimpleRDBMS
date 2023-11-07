import random
f=open("../data/input.txt","w+")
a=list(range(1,101))
random.shuffle(a)
for i in range(len(a)):
    string="INSERT "+str(a[i])+"\n"
    f.write(string)
for i in range(1,100):
    string="RANGE "+str(i)+" "+str(i+1)+"\n"
    f.write(string)
f.close()