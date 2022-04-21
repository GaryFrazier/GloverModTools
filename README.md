
# GloverModTools

Mod tools for the 2022 Steam port of Glover. Used to help with model creation for use with the [mod manager](https://github.com/GaryFrazier/GloverModManager) 

This tool converts .glo files (object files) into text format, and vice versa. This allows you to edit existing models, or create new ones.

Please post new mods on the reddit or discord.

https://www.reddit.com/r/glovermods/
https://discord.gg/6yJJThVdZm

## Installation
Download from the releases page https://github.com/GaryFrazier/GloverModTools/releases
Extract the folder anywhere you desire.

## Usage
This is a command line tool, if you need a visual application, leave an issue on the github and I will prioritize making one.

### Glo 2 Txt
To convert a Glover Object File (.glo) do the following:

Open a command prompt in the folder of the mod tool exe.

Place the .glo file you want to convert in this directory.

Run this command:

    ./gloverModTools glo2txt FILE_NAME.glo OUTPUT_FILE_NAME.txt
Where FILE_NAME.glo is your .glo file name you are converting and OUTPUT_FILE_NAME.txt is the name of the converted file you want created, which can be anything.

### Txt 2 Glo
To convert a txt file to a .glo object is similar to the above:

    ./gloverModTools txt2glo FILE_NAME.txt OUTPUT_FILE_NAME.glo
Essentially the same but you are using the existing txt file as FILE_NAME and it will create a .glo file as what you put for OUTPUT_FILE_NAME

Now you can replace the .glo file in the game directory with your new one, or create a mod package and use the mod manager (see above).