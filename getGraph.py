from sys import argv
import urllib.request

from pylab import figure, plot, grid, show


def meritevFunc(meritev):

    list_char = list(meritev)
    end_str = ""
    for ch in list_char:
        if ch.isdigit():
            str = ch + "0kV "
        else:
            str = ch.upper() + " "
        end_str = end_str + str
    
    print(end_str) 
    return end_str

def lisa_mer(contents):
    list_con = ""
    contents = str(contents)
    contents = contents[2:]
    contents = contents[:-1]
    contents = contents.split()
    print(contents)

    num = 0
    a = 0
    graphData = []
    asciData = [] 
    for elm in contents:
        asciData.append(chr(int(elm)))
        
    print(asciData)
    while len(asciData) != num:
        one = asciData[num]
        two = asciData[num+1]
        print(a)
        graphData.append(one + two)
        a = a + 1
        num = num + 2

    print(graphData)
    graphDataNum = []
    for elm in graphData:
        graphDataNum.append(int(elm, 16))

    print(graphDataNum)
    print("len. ", len(graphDataNum))
    
    data1 = graphDataNum[:64]
    print(len(data1))
    print(data1)
    data2 = graphDataNum[64:64*2]
    data3 = graphDataNum[64*2:64*3]
    data4 = graphDataNum[64*3:64*4]
    print(len(data2))
    print(len(data3))
    print(len(data4))
    print(data2)
    print(data3)
    print(data4)


    x = range(64)
    print(x)

    figure(1)
    plot(x, data1, x, data2, x, data3, x, data4)
    show()


script, file_path, meritev = argv

contents = urllib.request.urlopen("http://192.168.1.51/g").read()
print(contents)
str2 = lisa_mer(contents)

'''
#contents = "0, 1, 2, 0, 4, 0, 33506"
str1 = meritevFunc(meritev)
str2 = lisa_mer(contents)
str = str1 + " " + str2
printToFile(file_path, str)
'''

print("OUT")
print(str)
