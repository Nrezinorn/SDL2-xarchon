#pragma once
#if __APPLE__
#include <SDL2_mixer/SDL_mixer.h>
#else
#include "SDL2/SDL_mixer.h"
#endif
#include "Resource.h"
#include <string>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <iostream>
using std::string;
using std::stringstream;
using std::vector;

bool Resource::FileExists(const string& filePath) {
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

void Resource::SetName(const string& filePath) {
    vector<string> tokens;
    stringstream tokenizer(filePath.c_str());
    string intermediate;
    while(getline(tokenizer, intermediate, '/')) {
        tokens.push_back(intermediate);
    }
    this->name = tokens[tokens.size()-1];
}

string& Resource::GetName() {
    return this->name;
}

string& Resource::GetHash() {
    return this->hash;
}

Sound::Sound(const string& filePath) {
    // make sure that our file exists, cause that's a thing we should do
    if(!this->FileExists(filePath)) {
        std::cout << "File " << filePath << " does not exist." << std::endl;
        return;
    }
    this->filePath = filePath;
    //this->name = std::filesystem::path(filePath).filename().string();
    this->SetName(filePath);
    this->hash = "";
    this->sound = Mix_LoadWAV(this->filePath.c_str());
}

Sound::Sound(const string& filePath, const string& name = "") {
    // make sure that our file exists, cause that's a thing we should do
    if(!this->FileExists(filePath)) {
        std::cout << "File " << filePath << " does not exist." << std::endl;
        return;
    }
    this->filePath = filePath;
    this->name = name;
    this->hash = "";
    this->sound = Mix_LoadWAV(this->filePath.c_str());
}

Sound::~Sound() {
    if(!this->sound) {
        return;
    }
    Mix_FreeChunk(this->sound);
}

Mix_Chunk* Sound::GetSound() {
    return this->sound;
}

Mix_Chunk* Sound::GetAudio() {
    return this->GetSound();
}

Music::Music(const string& filePath) {
    // make sure that our file exists, cause that's a thing we should do
    if(!this->FileExists(filePath)) {
        std::cout << "File " << filePath << " does not exist." << std::endl;
        return;
    }
    this->SetName(filePath);
    this->hash = "";
    this->music = Mix_LoadMUS(this->filePath.c_str());
}

Music::Music(const string& filePath, const string& name = "") {
    // make sure that our file exists, cause that's a thing we should do
    if(!this->FileExists(filePath)) {
        std::cout << "File " << filePath << " does not exist." << std::endl;
        return;
    }
    this->name = name;
    this->hash = "";
    this->music = Mix_LoadMUS(this->filePath.c_str());
}

Music::~Music() {
    if(!this->music) {
        return;
    }
    Mix_FreeMusic(this->music);
}

Mix_Music* Music::GetMusic() {
    return this->music;
}

Mix_Music* Music::GetAudio() {
    return this->GetMusic();
}