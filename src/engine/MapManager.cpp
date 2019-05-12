/* OpenHoW
 * Copyright (C) 2017-2019 Mark Sowden <markelswo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <PL/platform_filesystem.h>

#include "engine.h"
#include "MapManager.h"
#include "Map.h"
#include "script/ScriptConfig.h"

#include "game/Game.h"
#include "game/BaseGameMode.h"

MapManager *MapManager::instance_ = nullptr;

MapManager::MapManager() {
    plRegisterConsoleCommand("map", MapCommand, "");
    plRegisterConsoleCommand("maps", MapsCommand, "");
}

MapManager::~MapManager() {
    manifests_.clear();
}

void MapManager::RegisterManifest(const std::string &path) {
    LogInfo("Registering map \"%s\"...\n", path.c_str());

    MapManifest manifest;
    manifest.path = path;
    try {
        ScriptConfig config(path);
        manifest.name           = config.GetStringProperty("name");
        manifest.author         = config.GetStringProperty("author");
        manifest.description    = config.GetStringProperty("description");

        manifest.sky            = config.GetStringProperty("sky");
        manifest.modes          = config.GetArrayStrings("modes");

        manifest.ambient_colour = config.GetColourProperty("ambient_colour", PLColour(255, 255, 255));
        manifest.sun_colour     = config.GetColourProperty("sun_colour", PLColour(255, 255, 255));
        manifest.sun_yaw        = config.GetFloatProperty("sun_yaw");
        manifest.sun_pitch      = config.GetFloatProperty("sun_pitch");

        manifest.temperature    = config.GetStringProperty("temperature", "hot");
        manifest.time           = config.GetStringProperty("time", "day");
    } catch(const std::exception &e) {
        LogWarn("Failed to read map config, \"%s\"!\n%s\n", path.c_str(), e.what());
    }

    char temp_buf[64];
    plStripExtension(temp_buf, sizeof(temp_buf), plGetFileName(path.c_str()));

    manifests_.insert(std::make_pair(temp_buf, manifest));
}

static void RegisterManifestInterface(const char *path) {
    MapManager::GetInstance()->RegisterManifest(path);
}

void MapManager::RegisterManifests() {
    manifests_.clear();

    std::string scan_path = std::string(GetBasePath()) + "/campaigns/" + GetCampaignPath() + "/maps";
    plScanDirectory(scan_path.c_str(), "map", RegisterManifestInterface, false);
}

const MapManifest *MapManager::GetManifest(const std::string &name) {
    auto manifest = manifests_.find(name);
    if(manifest != manifests_.end()) {
        return &manifest->second;
    }

    LogWarn("Failed to get manifest for \"%s\"!\n", name.c_str());
    return nullptr;
}

void MapManager::MapCommand(unsigned int argc, char **argv) {
    if(argc < 2) {
        LogWarn("Invalid number of arguments, ignoring!\n");
        return;
    }

    const MapManifest *desc = GetInstance()->GetManifest(argv[1]);
    if(desc == nullptr) {
        LogWarn("Failed to find manifest for \"%s\"!\n", argv[1]);
        return;
    }

    if(desc->modes.empty()) {
        LogWarn("No modes specified for \"%s\"!\n", argv[1]);
        return;
    }

    // set it to the first mode in the list for now
    if(!SetGameMode(desc->modes[0])) {
        return;
    }

    // now start it up!
    GetGameMode()->StartMode(argv[1]);
}

void MapManager::MapsCommand(unsigned int argc, char **argv) {
    if(GetInstance()->manifests_.empty()) {
        LogWarn("No maps available!\n");
        return;
    }

    for(auto manifest : GetInstance()->manifests_) {
        MapManifest *desc = &manifest.second;
        std::string out =
                desc->name + "/" + manifest.first +
                " : " + desc->description +
                " :";
        // print out all the supported modes
        for(size_t i = 0; i < desc->modes.size(); ++i) {
            out += (i == 0 ? " " : ", ") + desc->modes[i];
        }
        LogInfo("%s\n", out.c_str());
    }

    LogInfo("%u maps\n", GetInstance()->manifests_.size());
}