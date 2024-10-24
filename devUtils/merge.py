import os
from datetime import date

cppFiles = ["Core", "ErrorReporting", "Animations", "Texture", "Shader", "Camera", "GraphicModel",\
     "gl3d"]

headerFiles = ["Core", "ErrorReporting", "Animations", "json", "stb_image_write", "tiny_gltf", \
    "OBJ_Loader", "Texture", "Shader", "Camera", "GraphicModel",\
     "gl3d"]


initialtext = \
"""////////////////////////////////////////////////
//gl3D --Vlad Luta -- 
//built on {date}
////////////////////////////////////////////////
"""

perFileText = \
"""////////////////////////////////////////////////
//{file}
////////////////////////////////////////////////
"""


os.chdir("../")

os.chdir("src/shaders")


#shaders
shaderDir = os.getcwd()

os.chdir("../../")
os.chdir("devUtils")

devUtilsPath = os.getcwd()
shadersNames = []

for (root,dirs,f) in os.walk(shaderDir):
        #print (root)
        #print (dirs)
        #print (f)
        for file in f:
            fullPath = root + '/' + file
            #print(fullPath)
            os.system("glslOptimizer.exe " + fullPath)
            optimizedName = fullPath.split('.')
            optimizedName = optimizedName[0] + "_optimized." + optimizedName[1]
            
            optimizedFileName = file.split('.')
            optimizedFileName = optimizedFileName[0] + "_optimized." + optimizedFileName[1]

            os.replace(optimizedName, devUtilsPath + '/shaders/' + optimizedFileName)

            shadersNames.append(optimizedFileName)

os.chdir("../")


#print("working dir:")
#print(os.getcwd())

#.h
finalHFile = open("headerOnly/gl3d.h", "w")

finalHFile.write(initialtext.format(date = date.today()))
finalHFile.write("\n\n")

for i in headerFiles:
    f = open(os.path.join("src", "src", i + ".h"))

    content = f.read()

    for j in headerFiles:
        content = content.replace(f"#include <{j}.h>","")
        content = content.replace(f"#include<{j}.h>","")
        content = content.replace(f"#include \"{j}.h\"","")
        content = content.replace(f"#include\"{j}.h\"","")

    finalHFile.write(perFileText.format(file = i+".h"))
    finalHFile.write("#pragma region " + i + '\n')
    finalHFile.write(content)
    finalHFile.write("\n#pragma endregion" + '\n')

    finalHFile.write("\n\n")

    f.close()

finalHFile.close()


#.cpp
finalCppFile = open("headerOnly/gl3d.cpp", "w")

finalCppFile.write(initialtext.format(date = date.today()))
finalCppFile.write("\n")
finalCppFile.write("""#include \"gl3d.h\"""")
finalCppFile.write("\n\n")

finalCppFile.write("""
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE  
""")

headerOnlyFiles = ["json.h", "stb_image_write.h", "tiny_gltf.h"]

for i in headerOnlyFiles:
    f = open(os.path.join("src", "src", i))
    content = f.read()
    for j in headerFiles:
        content = content.replace(f"#include <{j}.h>","")
        content = content.replace(f"#include<{j}.h>","")
        content = content.replace(f"#include \"{j}.h\"","")
        content = content.replace(f"#include\"{j}.h\"","")
    finalCppFile.write(content)
    finalCppFile.write("\n\n")
    f.close()

for i in cppFiles:
    f = open(os.path.join("src", "src", i + ".cpp"))

    content = f.read()

    for j in headerFiles:
        content = content.replace(f"#include <{j}.h>","")
        content = content.replace(f"#include<{j}.h>","")
        content = content.replace(f"#include \"{j}.h\"","")
        content = content.replace(f"#include\"{j}.h\"","")

  

    if(i == "Shader"):
        newContent = content.split("\n")
        foundLineNr = 0
        for lineNr in range(len(newContent)):
            if(newContent[lineNr].find("//#pragma shaderSources") != -1):
                foundLineNr = lineNr
                break
        
        foundLineNr = foundLineNr+1

        newContent.insert(foundLineNr, "        //std::pair test stuff...")
        #todo add at foundLine nr the std::pair stuff

        stringToInsert = """name}", "{value}"}"""

        for shader in shadersNames:
            noOptimizedName = shader.replace("_optimized", "")
            shaderFile = open("devutils/shaders/" + shader)
            shaderSource = shaderFile.read()

            shaderSource = shaderSource.split('\n')
            
            for index in range(len(shaderSource)):
                shaderSource[index] = shaderSource[index].lstrip()
                if(shaderSource[index].startswith("//")):
                    shaderSource[index] = ""   
                shaderSource[index] = shaderSource[index].replace("#pragma debug(on)", "")
                found = shaderSource[index].find("//")
                if found >= 0:
                    shaderSource[index] = shaderSource[index][0:found]
                
                if(len(shaderSource[index]) != 0):
                    shaderSource[index] = "\"" + shaderSource[index] + "\\n\""

            shaderSource = list(filter(None, shaderSource))

            shaderSource = '\n'.join(shaderSource)

            #newContent.insert(foundLineNr, "      std::pair<std::string, const char*>{\""+noOptimizedName+"\", R\"(" + shaderSource +  ")\"},\n")
            newContent.insert(foundLineNr, "      std::pair<std::string, const char*>{\""+noOptimizedName+"\", " + shaderSource +  "},\n")
            shaderFile.close()

        newContent.insert(0, "#define GL3D_LOAD_SHADERS_FROM_HEADER_ONLY")
        content = '\n'.join(newContent)

    
    finalCppFile.write(perFileText.format(file = i+".cpp"))
    finalCppFile.write("#pragma region " + i + '\n')
    finalCppFile.write(content)
    finalCppFile.write("\n#pragma endregion" + '\n')

    finalCppFile.write("\n\n")

    f.close()

finalCppFile.close()