import os
from datetime import date

files = ["Core", "Texture", "Shader", "Camera", "GraphicModel",\
     ]

initialtext = \
"""////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on {date}
////////////////////////////////////////////////
"""

perFileText = \
"""////////////////////////////////////////////////
//{file}
////////////////////////////////////////////////
"""


os.chdir("../")

#.h
finalHFile = open("headerOnly/gl3d.h", "w")

finalHFile.write(initialtext.format(date = date.today()))
finalHFile.write("\n\n")

for i in files:
    f = open(os.path.join("gl3d", "src", i + ".h"))

    content = f.read()

    finalHFile.write(perFileText.format(file = i+".h"))
    finalHFile.write("#pragma region " + i + '\n')
    finalHFile.write(content)
    finalHFile.write("\n#pragma endregion" + '\n')

    finalHFile.write("\n\n")

    f.close()

finalHFile.close()


#.cpp
finalHFile = open("headerOnly/gl3d.cpp", "w")

finalHFile.write(initialtext.format(date = date.today()))
finalHFile.write("\n")
finalHFile.write("""#include \"gl3d.h\"""")
finalHFile.write("\n\n")


for i in files:
    f = open(os.path.join("gl3d", "src", i + ".cpp"))

    content = f.read()

#todo regex
    for j in files:
        content = content.replace(f"#include <{j}.h>","")
        content = content.replace(f"#include<{j}.h>","")
        content = content.replace(f"#include \"{j}.h\"","")
        content = content.replace(f"#include\"{j}.h\"","")

    finalHFile.write(perFileText.format(file = i+".cpp"))
    finalHFile.write("#pragma region " + i + '\n')
    finalHFile.write(content)
    finalHFile.write("\n#pragma endregion" + '\n')

    finalHFile.write("\n\n")

    f.close()

finalHFile.close()