/* OpenHoW
 * Copyright (C) 2017-2020 TalonBrave.info and Others (see CONTRIBUTORS)
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
#include "language.h"

#include "script/json_reader.h"

#define LNG_MANIFEST_PATH    "languages.manifest"

LanguageManager* LanguageManager::language_manager_;

LanguageManager::LanguageManager() {
	LogInfo( "Loading "
				 LNG_MANIFEST_PATH
				 "\n" );

	// Load in the languages manifest
	try {
		JsonReader manifest( LNG_MANIFEST_PATH );
		unsigned int num_keys = manifest.GetArrayLength();
		for ( unsigned int i = 0; i < num_keys; ++i ) {
			Index index;
			manifest.EnterChildNode( i );
			index.name = manifest.GetStringProperty( "name" );
			//LogDebug( "Language Name: %s\n", index.name.c_str() );
			index.key = manifest.GetStringProperty( "key" );
			//LogDebug( "Language Key: %s\n", index.key.c_str() );
			//index.font = manifest.GetStringProperty("font");
			//LogDebug("Language Font: %s\n", index.font.c_str());
			manifest.LeaveChildNode();
			languages_.insert( std::pair<std::string, Index>( index.key, index ) );
		}
	} catch ( const std::exception& e ) {
		Error( "Failed to load languages manifest, \"%s\"!\n", LNG_MANIFEST_PATH );
	}
}

LanguageManager::~LanguageManager() = default;

const char* LanguageManager::GetTranslation( const char* key ) { // todo: UTF-8 pls
	if ( key[ 0 ] != '$' ) {
		return key;
	}

	const char* p = ++key;
	if ( *p == '\0' ) {
		LogWarn( "Invalid key provided\n" );
		return p;
	}

	if ( current_language == nullptr ) {
		LogWarn( "No valid language set\n" );
		return p;
	}

	auto i = current_language->keys.find( p );
	if ( i == current_language->keys.end() ) {
		LogWarn( "Failed to find translation key, %s\n", p );
		return p;
	}

	return i->second.translation.c_str();
}

void LanguageManager::SetLanguage( const char* key ) {
	if ( current_language != nullptr && current_language->key == key ) {
		return;
	}

	auto language = languages_.find( key );
	if ( language == languages_.end() ) {
		Error( "Failed to find specified language, \"%s\"!\n", key );
	}

	current_language = &language->second;

	std::string filePath = "languages/" + current_language->key + ".language";
	try {
		JsonReader manifest( filePath );
		std::list<std::string> keys = manifest.GetObjectKeys();
		for ( const auto& idx : keys ) {
			current_language->keys.insert( std::pair<std::string, Key>( idx, {
				manifest.GetStringProperty( idx )
			} ) );
			//LogDebug( "Key: %s", idx.c_str() );
		}
	} catch ( const std::exception& e ) {
		Error( "Failed to load language manifest, \"%s\"!\n", filePath.c_str() );
	}
}

void LanguageManager::SetLanguageCallback( const PLConsoleVariable* var ) {
	LanguageManager::GetInstance()->SetLanguage( var->s_value );
}
