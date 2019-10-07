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

#pragma once

#include "../../property.h"

enum ActorFlag {
  ACTOR_FLAG_PLAYABLE = 1,
  ACTOR_FLAG_SCRIPTED = 16,
  ACTOR_FLAG_INSIDE = 32,
  ACTOR_FLAG_DELAYED = 64,
  /* ...any new types below... */
};

enum ActorEvent {
  ACTOR_EVENT_NONE,                       // does nothing!
  ACTOR_EVENT_ITEM,                       // spawns an item on destruction
  ACTOR_EVENT_PROMOTION,                  // spawns a promotion point on destruction
  ACTOR_EVENT_PROTECT = 4,                // spawns a promotion point if the object is not destroyed
  ACTOR_EVENT_AIRDROP_ITEM = 7,           // spawns item airdrop on destruction
  ACTOR_EVENT_AIRDROP_PROMOTION,          // spawns promotion point on destruction
  ACTOR_EVENT_GROUP_ITEM = 13,            // spawns item when group is destroyed
  ACTOR_EVENT_GROUP_PROMOTION,            // spawns promotion point when group is destroyed
  ACTOR_EVENT_REWARD = 19,                // returns specified item on destruction to destructor
  ACTOR_EVENT_GROUP_SPAWN,                // spawns group on destruction
  ACTOR_EVENT_VICTORY,                    // triggers victory on destruction
  ACTOR_EVENT_BURST,                      // spawns group upon destruction by TNT
  ACTOR_EVENT_GROUP_OBJECT,               // spawns group when object's group is destroyed
  /* ...any new types below... */
};

struct ActorSpawn {
  std::string class_name;
  PLVector3 position; // position in the world
  uint16_t index;                  // todo
  PLVector3 angles; // angles in the world
  uint16_t type;                   // todo

  int16_t bounds[3]; // collision bounds
  uint16_t bounds_type; // box, prism, sphere and none

  int16_t energy;
  uint8_t appearance;
  uint8_t team; // uk, usa, german, french, japanese, soviet

  uint16_t objective;
  uint8_t objective_actor_id;
  uint8_t objective_extra[2];

  PLVector3 fallback_position;

  int16_t extra;
  ActorSpawn* attachment{nullptr};
};

#define ACTOR_IMPLEMENT_SUPER(a) typedef a SuperClass;

class Player;

class Actor: public PropertyOwner {
 public:

  Actor();
  virtual ~Actor();

  virtual void Tick() {}  // simulation tick, called per-frame
  virtual void Draw() {}  // draw tick, called per-frame

  virtual void AddHealth(int16_t health);
  int16_t GetHealth() { return health_; }

  virtual bool IsVisible() { return is_visible_; }

  virtual PLVector3 GetPosition() { return position_; }
  virtual void SetPosition(PLVector3 position);

  virtual PLVector3 GetAngles() { return angles_; }
  virtual void SetAngles(PLVector3 angles);

  virtual bool Possessed(const Player* player);
  virtual void Depossessed(const Player* player);
  virtual void HandleInput();   // handle any player input, if applicable

  virtual ActorSpawn Serialize() { return ActorSpawn(); }
  virtual void Deserialize(const ActorSpawn& spawn);

  virtual void Activate() { is_activated_ = true; }
  virtual void Deactivate() { is_activated_ = false; }
  virtual bool IsActivated() { return is_activated_; }

  void DropToFloor();

 protected:
  bool is_visible_{false};

  float input_forward{0.00};  /* -1.0 = backwards, +1.0 = forwards */
  float input_yaw{0.00};      /* -1.0 = left, +1.0 = right */
  float input_pitch{0.00};    /* -1.0 = down, +1.0 = up */

  PLVector3 position_{0, 0, 0}, old_position_{0, 0, 0};
  PLVector3 fallback_position_{0, 0, 0};
  PLVector3 angles_{0, 0, 0}, old_angles_{0, 0, 0};
  PLVector3 bounds_{0, 0, 0};

  PLVector3 velocity_{0, 0, 0}, old_velocity_{0, 0, 0};

 private:
  uint16_t flags_{0};

  uint16_t team_{0};
  int16_t health_{0};

  bool is_activated_{false};

  Actor* parent_{nullptr};
  Actor* child_{nullptr};
};
