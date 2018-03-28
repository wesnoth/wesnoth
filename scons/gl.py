# vi: syntax=python:et:ts=4

def CheckOpenGL(context):
    context.Message("Checking for OpenGL... ")
    env = context.env
    backup = env.Clone().Dictionary()

    test_program = ""

    if env["PLATFORM"] == "win32":
        env.AppendUnique(LIBS = ["opengl32"])
        test_program += "#include <GL/opengl32.h>\n"
    elif env["PLATFORM"] == "darwin":
        env.AppendUnique(FRAMEWORKS = "OpenGL")
        test_program += "#include <OpenGL/gl.h>\n"
    else:
        env.AppendUnique(LIBS = ["GL"])
        test_program += "#include <GL/gl.h>\n"

    test_program += "int main()\n{}\n"

    if context.TryLink(test_program, ".c"):
        context.Result("yes")
        return True
    else:
        env.Replace(**backup)
        context.Result("no")
        return False

def CheckGLEW(context):
    context.Message("Checking for OpenGL Extension Wrangler... ")
    env = context.env
    backup = env.Clone().Dictionary()

    if env["PLATFORM"] == "win32":
        env.AppendUnique(LIBS = ["glew32", "opengl32"])
    elif env["PLATFORM"] == "darwin":
        env.AppendUnique(LIBS = ["GLEW"])
        env.AppendUnique(FRAMEWORKS = "OpenGL")
    else:
        env.AppendUnique(LIBS = ["GLEW", "GL"])

    test_program = """
        #include <GL/glew.h>
        int main()
        {
            glewInit();
        }
"""

    if context.TryLink(test_program, ".c"):
        context.Result("yes")
        return True
    else:
        env.Replace(**backup)
        context.Result("no")
        return False

config_checks = { "CheckOpenGL" : CheckOpenGL, "CheckGLEW" : CheckGLEW }
