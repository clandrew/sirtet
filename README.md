# Sirtet - Reverse Tetris

## About

Computers can be used for many things. Reading books, watching sports games, communicating with others. In your life, you may even come to play some games on your computer. Some of those games may have been good. Some may have been fun. This is not one of those.

This game plays like conventional Tetris, with one change- the 'camera', or viewing perspective. The 'camera' stays locked on to the piece in play; it's the grid that moves. The grid shifts around and rotates accordingly. To maintain the purity of this game concept, the controls are not affected by the changes in view.

## Controls
Up arrow key - Rotate the current piece

Left arrow key - Move the current piece one cell to the left, in world space

Right arrow key - Move the current piece one cell to the right, in world space

Down arrow key - Quick drop

Numerical key 1 - Toggle debug visuals (debug build only)

Numerical key 2 - Toggle the camera type between 'typical Tetris' and 'this Tetris' (debug build only)

Upon Game Over, press any key to start again.

## Demo

![Example image](https://raw.githubusercontent.com/clandrew/sirtet/master/Demo.gif "Example image")

## Build
This program is organized as a UWP application, built for x86-64 based architectures running a Windows 10 operating system. 2D graphics functionalities use Win2D.
