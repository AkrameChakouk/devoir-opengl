# Devoir OpenGL - scène de nuit

Auteur : Akrame  CHAKOUK

Projet du cours « Introduction to Modern OpenGL » (C++17, OpenGL 3.3, GLFW, GLEW, GLM).

- Fichier principal : `Devoir_Scene.cpp`
- Vidéo de démonstration : https://drive.google.com/file/d/1HTjmAJ92O-NgISw_oSoo8nl8ZC4YlW6M/view?usp=sharing

## Contenu de la scène
- Une place de nuit avec 20 objets (sol, lampadaires, tonneaux, caisses, robot, quilles, lapin, arbres).
- Trois types de lumières : directionnelle (lune), 3 ponctuelles colorées qui pulsent, spot (lampe torche).
- Animations : robot et lapin qui tournent, lumières qui pulsent, intensité de la lune qui varie.
- Visite virtuelle par une caméra automatique, ou caméra libre.

## Compilation (Linux)
    g++ -std=c++17 Devoir_Scene.cpp Camera.cpp Mesh.cpp ShaderProgram.cpp Texture2D.cpp -o Devoir_Scene -lglfw -lGLEW -lGL
    ./Devoir_Scene

Dépendances : libglfw3-dev, libglew-dev, libglm-dev.

## Touches
C : caméra libre / automatique, F : lampe torche, F1 : fil de fer, W A S D Z X + souris : déplacement en caméra libre, Échap : quitter.

## Crédits
- Moteur (Camera, Mesh, ShaderProgram, Texture2D), shaders et modèles de base : cours de Sébastien Mavromatis (AMU).
- Modèle « Tree » : Kenney, via Poly Pizza, licence CC0.
