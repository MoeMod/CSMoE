#pragma once

#include <string>

namespace xe {
    // If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
    inline bool CheckShader(GLuint handle, const char* desc)
    {
        GLint status = 0, log_length = 0;
        pglGetObjectParameterivARB(handle, GL_OBJECT_COMPILE_STATUS_ARB, &status);
        pglGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &log_length);
        if ((GLboolean)status == GL_FALSE)
            MsgDev( D_ERROR, "GL_Shader: failed to compile %s\n", desc);
        if (log_length > 1)
        {
            std::string buf;
            buf.resize(log_length);
            pglGetInfoLogARB(handle, log_length, NULL, (GLcharARB*)buf.data());
            MsgDev( D_ERROR, "%s\n", buf.begin());
        }
        return (GLboolean)status == GL_TRUE;
    }

    // If you get an error please report on GitHub. You may try different GL context version or GLSL version.
    inline bool CheckProgram(GLuint handle, const char* desc)
    {
        GLint status = 0, log_length = 0;
        pglGetObjectParameterivARB(handle, GL_OBJECT_LINK_STATUS_ARB, &status);
        pglGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &log_length);
        if ((GLboolean)status == GL_FALSE)
            MsgDev( D_ERROR, "GL_Shader: failed to link %s\n", desc);
        if (log_length > 1)
        {
            std::string buf;
            buf.resize(log_length);
            pglGetInfoLogARB(handle, log_length, NULL, (GLcharARB*)buf.data());
            MsgDev( D_ERROR, "%s\n", buf.begin());
        }
        return (GLboolean)status == GL_TRUE;
    }

    inline const char *GetShaderVersionString()
    {
#ifndef XASH_GLES
        return "#version 120\n";
#else
        return "#version 100\n";
#endif
    }
}