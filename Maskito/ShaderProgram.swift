//
//  ShaderProgram.swift
//

import Foundation

enum ShaderError: ErrorType {
    case COMPILE_ERROR, LINK_ERROR
}

class ShaderProgram {
    let programHandle : GLuint
    
    init?(withVertexShader vertexName: String, andFragmentShader fragmentName: String) {
        do {
            let vertexHandle = try ShaderProgram.compileShader(vertexName, shaderType: GLenum(GL_VERTEX_SHADER))
            let fragmentHandle = try ShaderProgram.compileShader(fragmentName, shaderType: GLenum(GL_FRAGMENT_SHADER))
            programHandle = try ShaderProgram.linkVertexShader(vertexHandle, fragmentShader: fragmentHandle)
        } catch {
            programHandle = 0
            return nil
        }
    }
    
    static func linkVertexShader(vertexShader : GLuint, fragmentShader : GLuint) throws -> GLuint {
        // Call glCreateProgram, glAttachShader, and glLinkProgram to link the vertex and fragment shaders into a complete program.
        let programHandle = glCreateProgram()
        glAttachShader(programHandle, vertexShader)
        glAttachShader(programHandle, fragmentShader)
        glLinkProgram(programHandle)
        
        // Check for any errors.
        var linkSuccess: GLint = GLint()
        glGetProgramiv(programHandle, GLenum(GL_LINK_STATUS), &linkSuccess)
        if (linkSuccess == GL_FALSE) {
            print("Failed to create shader program!")
            var infolog = [GLchar](count: 200, repeatedValue: 0)
            var length : GLsizei = GLsizei()
            glGetProgramInfoLog(programHandle, 200, &length, &infolog)
            print(String.fromCString(&infolog))
            throw ShaderError.LINK_ERROR
        }
        return programHandle
    }
    
    static func compileShader(shaderName: String, shaderType: GLenum) throws -> GLuint {
        
        // Get NSString with contents of our shader file.
        let shaderPath: String! = NSBundle.mainBundle().pathForResource(shaderName, ofType: "glsl")
        var shaderString: NSString?
        do {
            shaderString = try NSString(contentsOfFile:shaderPath, encoding: NSUTF8StringEncoding)
        } catch let error as NSError {
            print(error)
            shaderString = nil
        }
        if (shaderString == nil) {
            print("Failed to set contents shader of shader file!")
        }
        
        // Tell OpenGL to create an OpenGL object to represent the shader, indicating if it's a vertex or a fragment shader.
        let shaderHandle: GLuint = glCreateShader(shaderType)
        
        if shaderHandle == 0 {
            NSLog("Couldn't create shader")
        }
        // Conver shader string to CString and call glShaderSource to give OpenGL the source for the shader.
        var shaderStringUTF8 = shaderString!.UTF8String
        var shaderStringLength: GLint = GLint(Int32(shaderString!.length))
        glShaderSource(shaderHandle, 1, &shaderStringUTF8, &shaderStringLength)
        
        // Tell OpenGL to compile the shader.
        glCompileShader(shaderHandle)
        
        // But compiling can fail! If we have errors in our GLSL code, we can here and output any errors.
        var compileSuccess: GLint = GLint()
        glGetShaderiv(shaderHandle, GLenum(GL_COMPILE_STATUS), &compileSuccess)
        if (compileSuccess == GL_FALSE) {
            print("Failed to compile shader \(shaderName)!")
            // TODO: Actually output the error that we can get from the glGetShaderInfoLog function.
            var infolog = [GLchar](count: 100, repeatedValue: 0)
            var length : GLsizei = GLsizei()
            glGetShaderInfoLog(shaderHandle, 100, &length, &infolog)
            print(String.fromCString(&infolog))
            throw ShaderError.COMPILE_ERROR
        }
        return shaderHandle
    }
}