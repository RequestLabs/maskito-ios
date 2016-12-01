//
//  Shaders.swift
//

import Foundation

class ShaderManager {
    
    let passThroughShader : ShaderProgram
    let passThroughXYZ : GLuint
    let passThroughUV : GLuint
    let passThroughAlpha : GLuint
    let passThroughTex : GLint
    
    let basicShader : ShaderProgram
    let basicXYZ : GLuint
    let basicUV : GLuint
    let basicAlpha : GLuint
    let basicTex : GLint
    
    let hblurShader : ShaderProgram
    let hblurXYZ : GLuint
    let hblurUV : GLuint
    let hblurAlpha : GLuint
    let hblurScale : GLint
    let hblurTex : GLint
    
    let vblurShader : ShaderProgram
    let vblurXYZ : GLuint
    let vblurUV : GLuint
    let vblurAlpha : GLuint
    let vblurScale : GLint
    let vblurTex : GLint
    
    let havgShader : ShaderProgram
    let havgXYZ : GLuint
    let havgUV : GLuint
    let havgAlpha : GLuint
    let havgScale : GLint
    let havgTex : GLint
    
    let vavgShader : ShaderProgram
    let vavgXYZ : GLuint
    let vavgUV : GLuint
    let vavgAlpha : GLuint
    let vavgScale : GLint
    let vavgTex : GLint
    
    let satShader : ShaderProgram
    let satXYZ : GLuint
    let satUV : GLuint
    let satAlpha : GLuint
    let satScale : GLint
    let satTex : GLint
    
    let dentistShader : ShaderProgram
    let dentistXYZ : GLuint
    let dentistUV : GLuint
    let dentistBrighten : GLuint
    let dentistMaximum : GLint
    let dentistMinimum : GLint
    let dentistThreshold : GLint
    let dentistTex : GLint
    
    init?() {
        passThroughShader = ShaderProgram(withVertexShader: "TextureVertex", andFragmentShader: "TextureFragment")!
        passThroughXYZ = GLuint(glGetAttribLocation(passThroughShader.programHandle, "Position"))
        passThroughUV = GLuint(glGetAttribLocation(passThroughShader.programHandle, "TexSource"))
        passThroughAlpha = GLuint(glGetAttribLocation(passThroughShader.programHandle, "InAlpha"))
        passThroughTex = GLint(glGetUniformLocation(passThroughShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(passThroughXYZ)
        glEnableVertexAttribArray(passThroughUV)
        
        basicShader = ShaderProgram(withVertexShader: "PassVertex", andFragmentShader: "PassFragment")!
        basicXYZ = GLuint(glGetAttribLocation(basicShader.programHandle, "Position"))
        basicUV = GLuint(glGetAttribLocation(basicShader.programHandle, "TexSource"))
        basicAlpha = GLuint(glGetAttribLocation(basicShader.programHandle, "InAlpha"))
        basicTex = GLint(glGetUniformLocation(basicShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(basicXYZ)
        glEnableVertexAttribArray(basicUV)
        
        hblurShader = ShaderProgram(withVertexShader: "GaussianHorizontalVertex", andFragmentShader: "GaussianFragment")!
        hblurXYZ = GLuint(glGetAttribLocation(hblurShader.programHandle, "Position"))
        hblurUV = GLuint(glGetAttribLocation(hblurShader.programHandle, "TexSource"))
        hblurAlpha = GLuint(glGetAttribLocation(hblurShader.programHandle, "InAlpha"))
        hblurScale = GLint(glGetUniformLocation(hblurShader.programHandle, "Scale"))
        hblurTex = GLint(glGetUniformLocation(hblurShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(hblurXYZ)
        glEnableVertexAttribArray(hblurUV)
        
        vblurShader = ShaderProgram(withVertexShader: "GaussianVerticalVertex", andFragmentShader: "GaussianFragment")!
        vblurXYZ = GLuint(glGetAttribLocation(vblurShader.programHandle, "Position"))
        vblurUV = GLuint(glGetAttribLocation(vblurShader.programHandle, "TexSource"))
        vblurAlpha = GLuint(glGetAttribLocation(vblurShader.programHandle, "InAlpha"))
        vblurScale = GLint(glGetUniformLocation(vblurShader.programHandle, "Scale"))
        vblurTex = GLint(glGetUniformLocation(vblurShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(vblurXYZ)
        glEnableVertexAttribArray(vblurUV)

        havgShader = ShaderProgram(withVertexShader: "AverageHorizontalVertex", andFragmentShader: "AverageFragment")!
        havgXYZ = GLuint(glGetAttribLocation(havgShader.programHandle, "Position"))
        havgUV = GLuint(glGetAttribLocation(havgShader.programHandle, "TexSource"))
        havgAlpha = GLuint(glGetAttribLocation(havgShader.programHandle, "InAlpha"))
        havgScale = GLint(glGetUniformLocation(havgShader.programHandle, "Scale"))
        havgTex = GLint(glGetUniformLocation(havgShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(havgXYZ)
        glEnableVertexAttribArray(havgUV)

        vavgShader = ShaderProgram(withVertexShader: "AverageVerticalVertex", andFragmentShader: "AverageFragment")!
        vavgXYZ = GLuint(glGetAttribLocation(vavgShader.programHandle, "Position"))
        vavgUV = GLuint(glGetAttribLocation(vavgShader.programHandle, "TexSource"))
        vavgAlpha = GLuint(glGetAttribLocation(vavgShader.programHandle, "InAlpha"))
        vavgScale = GLint(glGetUniformLocation(vavgShader.programHandle, "Scale"))
        vavgTex = GLint(glGetUniformLocation(vavgShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(vavgXYZ)
        glEnableVertexAttribArray(vavgUV)
        
        satShader = ShaderProgram(withVertexShader: "DesaturateVertex", andFragmentShader: "DesaturateFragment")!
        satXYZ = GLuint(glGetAttribLocation(satShader.programHandle, "Position"))
        satUV = GLuint(glGetAttribLocation(satShader.programHandle, "TexSource"))
        satAlpha = GLuint(glGetAttribLocation(satShader.programHandle, "InAlpha"))
        satScale = GLint(glGetUniformLocation(satShader.programHandle, "scale"))
        satTex = GLint(glGetUniformLocation(satShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(satXYZ)
        glEnableVertexAttribArray(satUV)
        
        dentistShader = ShaderProgram(withVertexShader: "BrightenVertex", andFragmentShader: "BrightenFragment")!
        dentistXYZ = GLuint(glGetAttribLocation(dentistShader.programHandle, "Position"))
        dentistUV = GLuint(glGetAttribLocation(dentistShader.programHandle, "TexSource"))
        dentistBrighten = GLuint(glGetAttribLocation(dentistShader.programHandle, "InBrighten"))
        dentistMinimum = GLint(glGetUniformLocation(dentistShader.programHandle, "minimum"))
        dentistMaximum = GLint(glGetUniformLocation(dentistShader.programHandle, "maximum"))
        dentistThreshold = GLint(glGetUniformLocation(dentistShader.programHandle, "threshold"))
        dentistTex = GLint(glGetUniformLocation(dentistShader.programHandle, "TextureSampler"))
        glEnableVertexAttribArray(dentistXYZ)
        glEnableVertexAttribArray(dentistUV)
        
    }
    
    func activatePassThroughShader() -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(passThroughShader.programHandle)
        return (passThroughXYZ, passThroughUV, passThroughAlpha, passThroughTex)
    }
    
    func activateBasicShader() -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(basicShader.programHandle)
        return (basicXYZ, basicUV, basicAlpha, basicTex)
    }
    
    func activateHBlurShader(withScale scale : GLfloat) -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(hblurShader.programHandle)
        glUniform1f(hblurScale, scale)
        return (hblurXYZ, hblurUV, hblurAlpha, hblurTex)
    }
    
    func activateVBlurShader(withScale scale : GLfloat) -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(vblurShader.programHandle)
        glUniform1f(vblurScale, scale)
        return (vblurXYZ, vblurUV, vblurAlpha, vblurTex)
    }
    
    
    func activateHAvgShader(withScale scale : GLfloat) -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(havgShader.programHandle)
        glUniform1f(havgScale, scale)
        return (havgXYZ, havgUV, havgAlpha, havgTex)
    }
    
    func activateVAvgShader(withScale scale : GLfloat) -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(vavgShader.programHandle)
        glUniform1f(vavgScale, scale)
        return (vavgXYZ, vavgUV, vavgAlpha, vavgTex)
    }
    
    func activateSatShader(withScale scale : GLfloat) -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(satShader.programHandle)
        glUniform1f(satScale, scale)
        return (satXYZ, satUV, vavgAlpha, satTex)
    }
    
    func activateDentistShader(withMinimum min : GLfloat, andMaximum max: GLfloat, andThreshold threshold: GLfloat) -> (GLuint, GLuint, GLuint, GLint) {
        glUseProgram(dentistShader.programHandle)
        glUniform1f(dentistMinimum, min)
        glUniform1f(dentistMaximum, max)
        glUniform1f(dentistThreshold, threshold)
        return (dentistXYZ, dentistUV, dentistBrighten, dentistTex)
    }
    
    
}