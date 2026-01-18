#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "happly.h"
#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "car.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

// TODO: Il est fortement recommandé de définir quelques structs
//       pour représenter les attributs.
//       Faire de même pour représenter une vertex, qui est constitué d'attributs.
//       Cela facilitera l'utilisation et rendra votre code plus clair.
//       Un format entrelacé est recommandé (ordonné par vertex au lieu par attribut).
// struct ... { ... };

struct App : public OpenGLApplication
{
    App()
    : nSide_(5)
    , oldNSide_(0)
    , cameraPosition_(0.f, 0.f, 0.f)
    , cameraOrientation_(0.f, 0.f)
    , currentScene_(0)
    , isMouseMotionEnabled_(false)
    {
    }
	
	void init() override
	{
		// Le message expliquant les touches de clavier.
		setKeybindMessage(
			"ESC : quitter l'application." "\n"
			"T : changer de scène." "\n"
			"W : déplacer la caméra vers l'avant." "\n"
			"S : déplacer la caméra vers l'arrière." "\n"
			"A : déplacer la caméra vers la gauche." "\n"
			"D : déplacer la caméra vers la droite." "\n"
			"Q : déplacer la caméra vers le bas." "\n"
			"E : déplacer la caméra vers le haut." "\n"
			"Flèches : tourner la caméra." "\n"
			"Souris : tourner la caméra" "\n"
			"Espace : activer/désactiver la souris." "\n"
		);

		// Config de base.
		
		// TODO: Initialisez la couleur de fond.
       
        // TODO: Partie 2: Activez le test de profondeur (GL_DEPTH_TEST) et
        //       l'élimination des faces arrières (GL_CULL_FACE).
                
        loadShaderPrograms();
        
        // Partie 1
        initShapeData();
        
        // Partie 2
        loadModels();
        
        // TODO: Insérez les initialisations supplémentaires ici au besoin.
	}
	
	    
    void checkShaderCompilingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            glDeleteShader(id);
            std::cout << "Shader \"" << name << "\" compile error: " << infoLog << std::endl;
        }
    }


    void checkProgramLinkingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            glDeleteProgram(id);
            std::cout << "Program \"" << name << "\" linking error: " << infoLog << std::endl;
        }
    }
	

	// Appelée à chaque trame. Le buffer swap est fait juste après.
	void drawFrame() override
	{
	    // TODO: Nettoyage de la surface de dessin.
	    // TODO: Partie 2: Ajoutez le nettoyage du tampon de profondeur.
        
        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::End();
        
        switch (currentScene_)
        {
            case 0: sceneShape();  break;
            case 1: sceneModels(); break;
        }
	}

	// Appelée lorsque la fenêtre se ferme.
	void onClose() override
	{
	    // TODO: Libérez les ressources allouées (buffers, shaders, etc.).
	}

	// Appelée lors d'une touche de clavier.
	void onKeyPress(const sf::Event::KeyPressed& key) override
	{
		using enum sf::Keyboard::Key;
		switch (key.code)
		{
		    case Escape:
		        window_.close();
	        break;
		    case Space:
		        isMouseMotionEnabled_ = !isMouseMotionEnabled_;
		        if (isMouseMotionEnabled_)
		        {
		            window_.setMouseCursorGrabbed(true);
		            window_.setMouseCursorVisible(false);
	            }
	            else
	            {
	                window_.setMouseCursorGrabbed(false);
	                window_.setMouseCursorVisible(true);
                }
	        break;
	        case T:
                currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
            break;
		    default: break;
		}
	}

	void onResize(const sf::Event::Resized& event) override
	{	
	}
	
	void onMouseMove(const sf::Event::MouseMoved& mouseDelta) override
	{	    
	    if (!isMouseMotionEnabled_)
	        return;
        
        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
	    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
	}
	
	void updateCameraInput() 
    {
        if (!window_.hasFocus())
            return;
            
        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }
        
        float cameraMouvementX = 0;
        float cameraMouvementY = 0;
        
        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;
        
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;
            
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }
    
    void loadModels()
    {
        car_.loadModels();
        tree_.load("../models/pine.ply");
        streetlight_.load("../models/streetlight.ply");
        grass_.load("../models/grass.ply");
        street_.load("../models/street.ply");
        streetcorner_.load("../models/streetcorner.ply");
    }
    
    GLuint loadShaderObject(GLenum type, const char* path)
    {
        // TODO: Chargement d'un shader object.
        //       Utilisez readFile pour lire le fichier.
        //       N'oubliez pas de vérifier les erreurs suite à la compilation
        //       avec la méthode App::checkShaderCompilingError.
        
        return 0;
    }
    
    void loadShaderPrograms()
    {
        // TODO: Chargement des shader programs.
        //       N'oubliez pas de vérifier les erreurs suite à la liaison (linking)
        //       avec la méthode App::checkProgramLinkingError. Vous pouvez
        //       donner un nom unique pour plus facilement lire les erreurs 
        //       dans la console.
        //       Il est recommandé de détacher et de supprimer les shaders objects
        //       directement après la liaison.
        
        // Partie 1
        const char* BASIC_VERTEX_SRC_PATH = "./shaders/basic.vs.glsl";
        const char* BASIC_FRAGMENT_SRC_PATH = "./shaders/basic.fs.glsl";
        
        // Partie 2
        const char* TRANSFORM_VERTEX_SRC_PATH = "./shaders/transform.vs.glsl";
        const char* TRANSFORM_FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";
        
        // TODO: Allez chercher les locations de vos variables uniform dans le shader
        //       pour initialiser mvpUniformLocation_ et car_.mvpUniformLocation,
        //       puis colorModUniformLocation_ et car_.colorModUniformLocation.
    }
    
    void generateNgon()
    {
        // TODO: Générez un polygone à N côtés (couramment appelé N-gon).
        //       Vous devez gérer les cas entre 5 et 12 côtés (pentagone, hexagone
        //       , etc.). Ceux-ci ont un rayon constant de 0.7.
        //       Chaque point possède une couleur (libre au choix).
        //       Vous devez minimiser le nombre de points et définir des indices
        //       pour permettre la réutilisation.        
        
        const float RADIUS = 0.7f;
    }
    
    void initShapeData()
    {
        // TODO: Initialisez les objets graphiques pour le dessin du polygone.
        //       Ne passez aucune donnée pour le moment (déjà géré dans App::sceneShape),
        //       on demande seulement de faire l'allocation de buffers suffisamment gros
        //       pour contenir le polygone durant toute l'exécution du programme.
        //       Réfléchissez bien à l'usage des buffers (paramètre de glBufferData).
    
        // TODO: Créez un vao et spécifiez le format des données dans celui-ci.
        //       N'oubliez pas de lier le ebo avec le vao et de délier le vao
        //       du contexte pour empêcher des modifications sur celui-ci.
    }
    
    void sceneShape()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderInt("Sides", &nSide_, MIN_N_SIDES, MAX_N_SIDES);
        ImGui::End();
        
        bool hasNumberOfSidesChanged = nSide_ != oldNSide_;
        if (hasNumberOfSidesChanged)
        {
            oldNSide_ = nSide_;
            // generateNgon(vertices_, elements_, nSide_);
            
            // TODO: Le nombre de côtés a changé, la méthode App::generateNgon
            //       (que vous avez implémentée) a modifié les données sur le CPU.
            //       Ici, il faut envoyer les données à jour au GPU.
            //       Attention, il ne faut pas faire d'allocation/réallocation, on veut
            //       seulement mettre à jour les buffers actuels.
        }
        
        // TODO: Dessin du polygone.
    }
    
    void drawStreetlights(glm::mat4& projView)
    {
        // TODO: Dessin des lampadaires. Ceux-ci doivent être immobiles.
        //
        //       Placez les positions comme sur l'image de l'énoncé.
        //       La distance par rapport au bord de la route est de 0.5.
        //       La hauteur est de -0.15 (un peu renfoncé dans le sol).
        //
        //       Ils sont toujours orientés de façon perpendiculaire à la route
        //       pour "l'éclairer".
    }
    
    void drawTree(glm::mat4& projView)
    {
        // TODO: Dessin de l'arbre. Il doit être immobile.
        //
        //       Placez la position comme sur l'image de l'énoncé.
        //
        //       Il a une mise à l'échelle sur tous les axes
        //       d'un facteur 15.
        //
        //       Désactiver le CULL_FACE pour voir les faces arrières
        //       seulement pour ce dessin.
    }
    
    void drawGround(glm::mat4& projView)
    {
        // TODO: Dessin du sol.
        //
        //       Ici, les modèles originaux sont des carrés de 1 unité.
        //       
        //       La route est consistué par partie de plusieurs segments de route.
        //       
        //       La longueur total d'un côté de route est de 30 et il doit y avoir 7
        //       segments. Un segment de route est 5 unitées de large.
        //       La ligne blanche devrait traverser le long de la route.
        //
        //       Les coins des routes sont remplis par un autre modèle sans ligne blanche.
        //
        //       Le gazon a aussi une mise à l'échelle pour être long de 50
        //       unités et large de 50. Celui-ci doit aussi être légèrement en
        //       dessous de la route de 0.1.
        //       Réflexion supplémentaire: Que se passe-t-il s'il n'est pas déplacé?
        //       Comment expliquer ce qui est visible?
    }
    
    glm::mat4 getViewMatrix()
    {
        // TODO: Calculer la matrice de vue.
        //
        //       Vous n'avez pas le droit d'utiliser de fonction lookAt ou 
        //       d'inversion de matrice. À la place, procéder en inversant
        //       les opérations. N'oubliez pas que cette matrice est appliquée
        //       aux éléments de la scène. Au lieu de déplacer la caméra 10
        //       unités vers la gauche, on déplace le monde 10 unités vers la
        //       droite.
        //
        //       La caméra est placée à la position cameraPosition et orientée
        //       par les angles cameraOrientation (en radian).
        
        return glm::mat4(1.0);
    }
    
    glm::mat4 getPerspectiveProjectionMatrix()
    {
        // TODO: Calculer la matrice de projection.
        //
        //       Celle-ci aura un fov de 70 degrés, un near à 0.1 et un far à 300.
        //       
        
        // getWindowAspect();
        
        return glm::mat4(1.0);
    }
    
    void sceneModels()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderFloat("Car speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
        ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f, "%.2f°");
        if (ImGui::Button("Reset steering"))
            car_.steeringAngle = 0.f;
        ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
        ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
        ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
        ImGui::Checkbox("Brake", &car_.isBraking);
        ImGui::End();
    
        updateCameraInput();
        car_.update(deltaTime_);
        
        // TODO: Dessin de la totalité de la scène graphique.
        //       On devrait voir la route, le gazon, l'arbre, les lampadaires
        //       et la voiture. La voiture est contrôlable avec l'interface graphique.
    }
    
private:
    // Shaders
    GLuint basicSP_;
    GLuint transformSP_;
    GLuint colorModUniformLocation_;
    GLuint mvpUniformLocation_;
    
    // Partie 1
    GLuint vbo_, ebo_, vao_;
    
    static constexpr unsigned int MIN_N_SIDES = 5;
    static constexpr unsigned int MAX_N_SIDES = 12;
    
    // TODO: Modifiez les types de vertices_ et elements_ pour votre besoin.
    //  vertices_[MAX_N_SIDES + 1];
    //  elements_[MAX_N_SIDES * 3];
    
    int nSide_, oldNSide_;
    
    // Partie 2
    Model tree_;
    Model streetlight_;
    Model grass_;
    Model street_;
    Model streetcorner_;
    
    Car car_;
    
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;
    
    // Matrices statiques
    static constexpr unsigned int N_STREETLIGHTS = 2*4;
    static constexpr unsigned int N_STREET_PATCHES = 7*4+4;
    glm::mat4 treeModelMatrice_;
    glm::mat4 groundModelMatrice_;
    glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];
    glm::mat4 streetPatchesModelMatrices_[N_STREET_PATCHES];
    
    // Imgui var
    const char* const SCENE_NAMES[2] = {
        "Introduction",
        "3D Model & transformation",
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;
    
    bool isMouseMotionEnabled_;
};


int main(int argc, char* argv[])
{
	WindowSettings settings = {};
	settings.fps = 60;
	settings.context.depthBits = 24;
	settings.context.stencilBits = 8;
	settings.context.antiAliasingLevel = 4;
	settings.context.majorVersion = 3;
	settings.context.minorVersion = 3;
	settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

	App app;
	app.run(argc, argv, "Tp1", settings);
}
