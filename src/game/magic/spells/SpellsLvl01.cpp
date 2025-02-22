/*
 * Copyright 2014-2019 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game/magic/spells/SpellsLvl01.h"

#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/Cheat.h"
#include "game/NPC.h"
#include "game/effect/ParticleSystems.h"
#include "game/magic/spells/SpellsLvl03.h"

#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"

#include "physics/Collisions.h"

#include "scene/GameSound.h"
#include "scene/Object.h"

#include "util/Range.h"

bool MagicSightSpell::CanLaunch() {
	return spells.getSpellByCaster(m_caster, m_type) == nullptr;
}

void MagicSightSpell::Launch() {
	
	m_fManaCostPerSecond = 0.36f;
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_VISION_START, &m_caster_pos);
	
	if(m_caster == EntityHandle_Player) {
		player.m_improve = true;
		m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_VISION_LOOP, &m_caster_pos, 1.f);
	}
}

void MagicSightSpell::End() {
	
	if(m_caster == EntityHandle_Player) {
		player.m_improve = false;
	}
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	Entity * caster = entities.get(m_caster);
	if(caster) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_VISION_START, &caster->pos);
	}
}

void MagicSightSpell::Update() {
	
	if(m_caster == EntityHandle_Player) {
		Vec3f pos = ARX_PLAYER_FrontPos();
		ARX_SOUND_RefreshPosition(m_snd_loop, pos);
	}
}

static void LaunchMagicMissileExplosion(const Vec3f & _ePos, bool mrCheat) {
	
	ParticleParams cp = g_particleParameters[ParticleParam_MagicMissileExplosion];
	
	if(mrCheat) {
		cp = g_particleParameters[ParticleParam_MagicMissileExplosionMar];
	}
	
	ParticleSystem * pPS = new ParticleSystem();
	pPS->SetParams(cp);
	pPS->SetPos(_ePos);
	pPS->Update(0);
	
	EERIE_LIGHT * light = dynLightCreate();
	if(light) {
		light->intensity = 2.3f;
		light->fallstart = 250.f;
		light->fallend   = 420.f;

		if(mrCheat) {
			light->rgb = Color3f(1.f, 0.3f, .8f);
		} else {
			light->rgb = Color3f(0.f, 0.f, .8f);
		}

		light->pos = _ePos;
		light->duration = 1500ms;
	}
	
	g_particleManager.AddSystem(pPS);
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_MM_HIT, &_ePos);
}



MagicMissileSpell::MagicMissileSpell()
	: m_mrCheat(false)
{ }

void MagicMissileSpell::Launch() {
	
	m_duration = 6s;
	m_hasDuration = true;
	
	m_hand_group = GetActionPointIdx(entities[m_caster]->obj, "primary_attach");
	
	if(m_hand_group != ActionPoint()) {
		Entity * caster = entities[m_caster];
		ActionPoint group = m_hand_group;
		m_hand_pos = actionPointPosition(caster->obj, group);
	}
	
	Vec3f startPos = m_hand_pos;
	float pitch, yaw;
	if(m_caster == EntityHandle_Player) {
		pitch = player.angle.getPitch();
		yaw = player.angle.getYaw();
		if(m_hand_group == ActionPoint()) {
			startPos = player.pos + angleToVectorXZ(yaw);
		}
	} else {
		pitch = 0.f;
		yaw = entities[m_caster]->angle.getYaw();
		if(m_hand_group == ActionPoint()) {
			startPos = entities[m_caster]->pos;
		}
	}
	
	startPos += angleToVector(Anglef(pitch, yaw, 0.f)) * 60.f;
	
	if(m_caster != EntityHandle_Player) {
		Entity * io = entities[m_caster];
		if(Entity * entityTarget = entities.get(io->targetinfo)) {
			const Vec3f & p1 = m_caster_pos;
			const Vec3f & p2 = entityTarget->pos;
			pitch = -(glm::degrees(getAngle(p1.y, p1.z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1.x, p1.z)))));
		} else if(Entity * spellTarget = entities.get(m_target)) {
			const Vec3f & p1 = m_caster_pos;
			const Vec3f & p2 = spellTarget->pos;
			pitch = -(glm::degrees(getAngle(p1.y, p1.z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1.x, p1.z)))));
		}
	}
	
	m_mrCheat = (m_caster == EntityHandle_Player && cur_mr == 3);
	
	GameDuration lMax = 0;
	
	size_t number;
	if(sp_max || cur_rf == 3) {
		number = size_t(m_level);
	} else {
		number = glm::clamp<size_t>(size_t(m_level + 1) / 2, 1, 5);
	}
	
	m_lights.reserve(number);
	m_missiles.reserve(number);
	
	for(size_t i = 0; i < number; i++) {
		LightHandle lightHandle;
		EERIE_LIGHT * el = dynLightCreate(lightHandle);
		if(el) {
			el->intensity = 0.7f + 2.3f;
			el->fallend = 190.f;
			el->fallstart = 80.f;
			
			if(m_mrCheat) {
				el->rgb = Color3f(1.f, 0.3f, 0.8f);
			} else {
				el->rgb = Color3f(0.f, 0.f, 1.f);
			}
			
			el->pos = startPos;
			el->duration = 300ms;
		}
		m_lights.push_back(lightHandle);
	}
	
	for(size_t i = 0; i < number; i++) {
		
		if(!m_mrCheat) {
			m_missiles.emplace_back(std::make_unique<CMagicMissile>());
		} else {
			m_missiles.emplace_back(std::make_unique<MrMagicMissileFx>());
		}
		
		CMagicMissile & missile = *m_missiles.back();
		
		Anglef angles(pitch, yaw, 0.f);
		
		if(i > 0) {
			angles.setPitch(angles.getPitch() + Random::getf(-4.0f, 4.0f));
			angles.setYaw(angles.getYaw() + Random::getf(-6.0f, 6.0f));
		}
		
		missile.Create(startPos, angles);
		
		GameDuration lTime = m_duration + std::chrono::milliseconds(Random::get(-1000, 1000));
		
		lTime = std::max(GameDuration(1s), lTime);
		lMax = std::max(lMax, lTime);
		
		missile.SetDuration(lTime);
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_MM_CREATE, &startPos);
	ARX_SOUND_PlaySFX(g_snd.SPELL_MM_LAUNCH, &startPos);
	snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_MM_LOOP, &startPos, 1.f);
	
	m_duration = lMax + 1s;
}

void MagicMissileSpell::End() {
	
	for(LightHandle light : m_lights) {
		endLightDelayed(light, 500ms);
	}
	
	m_missiles.clear();
	
	ARX_SOUND_Stop(snd_loop);
	snd_loop = audio::SourcedSample();
}

void MagicMissileSpell::Update() {
	
	for(CMagicMissile & missile : util::dereference(m_missiles)) {
		
		if(missile.bExplo) {
			continue;
		}
			
		Sphere sphere = Sphere(missile.eCurPos, 10.f);
		
		Entity * caster = entities.get(m_caster);
		if(CheckAnythingInSphere(sphere, caster, CAS_NO_SAME_GROUP)) {
			
			LaunchMagicMissileExplosion(missile.eCurPos, m_mrCheat);
			if(caster) {
				spawnAudibleSound(missile.eCurPos, *caster);
			}
			
			missile.SetTTL(1s);
			missile.bExplo = true;
			missile.bMove  = false;
			
			DamageParameters damage;
			damage.pos = missile.eCurPos;
			damage.radius = 80.f;
			damage.damages = (4 + m_level * 0.2f) * 0.8f;
			damage.area = DAMAGE_FULL;
			damage.duration = GameDuration::ofRaw(-1);
			damage.source = m_caster;
			damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
			damage.type = DAMAGE_TYPE_MAGICAL;
			DamageCreate(damage);
			
			Color3f rgb(.3f, .3f, .45f);
			ARX_PARTICLES_Add_Smoke(missile.eCurPos, 0, 6, rgb);
		}
	}
	
	Vec3f averageMissilePos = Vec3f(0.f);
	
	for(CMagicMissile & missile : util::dereference(m_missiles)) {
		missile.Update(g_gameTime.lastFrameDuration());
		averageMissilePos += missile.eCurPos;
	}
	
	averageMissilePos /= float(m_missiles.size());
	ARX_SOUND_RefreshPosition(snd_loop, averageMissilePos);
	
	arx_assert(m_lights.size() == m_missiles.size());
	
	for(size_t i = 0; i < m_lights.size(); i++) {
		EERIE_LIGHT * light = lightHandleGet(m_lights[i]);
		if(light) {
			light->intensity = 0.7f + 2.3f * Random::getf(0.5f, 1.0f);
			light->pos = m_missiles[i]->eCurPos;
			light->creationTime = g_gameTime.now();
		}
	}
	
	{
		long nbmissiles = 0;
		for(CMagicMissile & missile : util::dereference(m_missiles)) {
			if(missile.bMove) {
				nbmissiles++;
			}
		}
		if(nbmissiles == 0) {
			requestEnd();
		}
	}
	
	for(CMagicMissile & missile : util::dereference(m_missiles)) {
		missile.Render();
	}
	
}


IgnitSpell::IgnitSpell()
	: m_srcPos(0.f)
{ }

void IgnitSpell::Launch() {
	
	m_duration = 500ms;
	m_hasDuration = true;
	
	if(m_hand_group != ActionPoint()) {
		m_srcPos = m_hand_pos;
	} else {
		m_srcPos = m_caster_pos - Vec3f(0.f, 50.f, 0.f);
	}
	
	if(EERIE_LIGHT * light = dynLightCreate()) {
		light->intensity = 1.8f;
		light->fallend   = 450.f;
		light->fallstart = 380.f;
		light->rgb       = Color3f(1.f, 0.75f, 0.5f);
		light->pos       = m_srcPos;
		light->duration  = 300ms;
	}
	
	float fPerimeter = 400.f + m_level * 30.f;
	
	m_lights.clear();
	
	igniteEntities(Sphere(m_srcPos, fPerimeter), true);
	
	for(size_t ii = 0; ii < g_staticLights.size(); ii++) {
		const EERIE_LIGHT * light = &g_staticLights[ii];
		
		if(!(light->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(m_caster == EntityHandle_Player && (light->extras & EXTRAS_NO_IGNIT)) {
			continue;
		}
		
		if(!(light->extras & EXTRAS_SEMIDYNAMIC)
		  && !(light->extras & EXTRAS_SPAWNFIRE)
		  && !(light->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(light->m_ignitionStatus) {
			continue;
		}
		
		if(!fartherThan(m_srcPos, light->pos, fPerimeter)) {
			
			T_LINKLIGHTTOFX entry;
			
			entry.m_targetLight = ii;
			
			EERIE_LIGHT * effectLight = dynLightCreate(entry.m_effectLight);
			if(effectLight) {
				effectLight->intensity = Random::getf(0.7f, 2.7f);
				effectLight->fallend = 400.f;
				effectLight->fallstart = 300.f;
				effectLight->rgb = Color3f(1.f, 1.f, 1.f);
				effectLight->pos = light->pos;
			}
			
			m_lights.push_back(entry);
		}
	}
	
	for(Spell & spell : spells.ofType(SPELL_FIREBALL)) {
		float radius = std::max(m_level * 2.f, 12.f); // TODO is this supposed to use spell.m_level?
		if(closerThan(m_srcPos, static_cast<FireballSpell &>(spell).getPosition(), fPerimeter + radius)) {
			spell.m_level += 1;
		}
	}
	
}

void IgnitSpell::End() {
	
	for(T_LINKLIGHTTOFX & entry : m_lights) {
		EERIE_LIGHT * light = &g_staticLights[entry.m_targetLight];
		light->m_ignitionStatus = true;
		ARX_SOUND_PlaySFX(g_snd.SPELL_IGNITE, &light->pos);
		lightHandleDestroy(entry.m_effectLight);
	}
	
	m_lights.clear();
	
}

void IgnitSpell::Update() {
	
	float a = m_elapsed / m_duration;
	
	if(a >= 1.f) {
		a = 1.f;
	}
	
	for(const T_LINKLIGHTTOFX & entry : m_lights) {
		EERIE_LIGHT * targetLight = &g_staticLights[entry.m_targetLight];
		EERIE_LIGHT * light = lightHandleGet(entry.m_effectLight);
		if(light) {
			light->intensity = Random::getf(0.7f, 2.7f);
			light->pos = glm::mix(m_srcPos, targetLight->pos, a);
		}
	}
	
}

void DouseSpell::Launch() {
	
	m_duration = 500ms;
	m_hasDuration = true;
	
	Vec3f target = (m_hand_group != ActionPoint()) ? m_hand_pos : (m_caster_pos - Vec3f(0.f, 50.f, 0.f));
	
	float fPerimeter = 400.f + m_level * 30.f;
	
	igniteEntities(Sphere(target, fPerimeter), false);
	
	for(size_t ii = 0; ii < g_staticLights.size(); ii++) {
		const EERIE_LIGHT * light = &g_staticLights[ii];
		
		if(!light || !(light->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(!(light->extras & EXTRAS_SEMIDYNAMIC)
		  && !(light->extras & EXTRAS_SPAWNFIRE)
		  && !(light->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(!light->m_ignitionStatus) {
			continue;
		}
		
		if(!fartherThan(target, light->pos, fPerimeter)) {
			m_lights.push_back(ii);
		}
	}
	
	if(player.torch && closerThan(target, player.pos, fPerimeter)) {
		ARX_PLAYER_ClickedOnTorch(player.torch);
	}
	
	for(Spell & spell : spells) {
		
		switch(spell.m_type) {
			
			case SPELL_FIREBALL: {
				Vec3f pos = spell.getPosition();
				float radius = std::max(m_level * 2.f, 12.f);
				if(closerThan(target, pos, fPerimeter + radius)) {
					spell.m_level -= m_level;
					if(spell.m_level < 1) {
						spells.endSpell(&spell);
					}
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				Vec3f pos = spell.getPosition();
				if(closerThan(target, pos, fPerimeter + 200)) {
					spell.m_level -= m_level;
					if(spell.m_level < 1) {
						spells.endSpell(&spell);
					}
				}
				break;
			}
			
			default: break;
		}
		
	}
	
}

void DouseSpell::End() {
	
	for(size_t index : m_lights) {
		EERIE_LIGHT * light = &g_staticLights[index];
		light->m_ignitionStatus = false;
		ARX_SOUND_PlaySFX(g_snd.SPELL_DOUSE, &light->pos);
	}
	
}

void DouseSpell::Update() {
	
}

void ActivatePortalSpell::Launch() {
	
	ARX_SOUND_PlayInterface(g_snd.SPELL_ACTIVATE_PORTAL);
	
	m_duration = 20ms;
	m_hasDuration = true;
}
