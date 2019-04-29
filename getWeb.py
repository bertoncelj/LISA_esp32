from sys import argv
import urllib.request

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
    contents =  str(contents)
    print(contents)
    list_con = contents.split(",")
    print(list_con)
    
    text = " ".join(list_con[1:])
    return str(text)

def printToFile(file_path, whole_stc):
    filename = file_path
    write_f = open(filename, 'a')
    
    write_f.write("%s\n" % whole_stc)
    
    write_f.close()


script, file_path, meritev = argv



contents = urllib.request.urlopen("http://192.168.1.51/data").read()

#contents = "0, 1, 2, 0, 4, 0, 33506"
str1 = meritevFunc(meritev)
str2 = lisa_mer(contents)
str = str1 + " " + str2
printToFile(file_path, str)


print("OUT")
print(str)
