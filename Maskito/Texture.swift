//
//  Texture.swift
//

import Foundation

class Texture {
    var name : GLuint = GLuint()
    var target : GLuint = GLuint()
    
    init() {
        // Create a texture in OpenGL land
        name = 0
        target = 0
    }
    
    func attachToTextureUnit() {
        
    }
    
}