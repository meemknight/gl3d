import os
from datetime import date

files = ["Shader", "Camera"]

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

finalHFile = open("headerOnly/gl3d.h", "w")

finalHFile.write(initialtext.format(date = date.today()))
finalHFile.write('\n')

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