/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Initial Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "graphics/effects/PolyBoom.h"

#include <array>

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Entity.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Draw.h"
#include "graphics/GlobalFog.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleTextures.h"
#include "graphics/texture/TextureStage.h"
#include "platform/profiler/Profiler.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

enum DecalType {
	ScorchMarkDecal,
	BloodDecal,
	WaterDecal
};

struct POLYBOOM {
	EERIEPOLY * ep;
	float u[4];
	float v[4];
	Color3f rgb;
	DecalType type;
	bool fastdecay;
	short nbvert;
	TextureContainer * tc;
	GameInstant timecreation;
	GameDuration tolive;
};

static const size_t MAX_POLYBOOM = 4000;
static std::vector<POLYBOOM> polyboom(MAX_POLYBOOM);

static const float BOOM_RADIUS = 420.f;

extern TextureContainer * Boom;

size_t PolyBoomCount() {
	return polyboom.size();
}

void PolyBoomClear() {
	polyboom.clear();
}

void PolyBoomAddScorch(const Vec3f & poss) {
	
	for(auto tile : g_tiles->tilesAround(g_tiles->getTile(poss), 3))  {
		for(EERIEPOLY & ep : tile.polygons()) {
			
			if((ep.type & POLY_TRANS) && !(ep.type & POLY_WATER)) {
				continue;
			}
			
			size_t nbvert = (ep.type & POLY_QUAD) ? 4 : 3;
			
			float temp_uv1[4];
			
			bool dod = true;
			for(size_t k = 0; k < nbvert; k++) {
				float ddd = fdist(ep.v[k].p, poss);
				if(ddd > BOOM_RADIUS) {
					dod = false;
					break;
				} else {
					temp_uv1[k] = 0.5f - ddd * (0.5f / BOOM_RADIUS);
				}
			}
			if(!dod) {
				continue;
			}
			
			if(polyboom.size() >= MAX_POLYBOOM) {
				continue;
			}
			
			POLYBOOM pb;
			
			pb.type = ScorchMarkDecal;
			pb.fastdecay = false;
			pb.ep = &ep;
			pb.tc = g_particleTextures.boom;
			pb.tolive = 10s;
			pb.timecreation = g_gameTime.now();
			pb.rgb = Color3f::black;
			for(size_t k = 0; k < nbvert; k++) {
				pb.v[k] = pb.u[k] = temp_uv1[k];
			}
			pb.nbvert = short(nbvert);
			
			polyboom.push_back(pb);
			
		}
	}
	
}

void PolyBoomAddSplat(const Sphere & sp, const Color3f & col, long flags) {
	
	Vec3f poss = sp.origin;
	float size = sp.radius;
	
	if(polyboom.size() > (MAX_POLYBOOM >> 2) - 30)
		return;
	
	if(polyboom.size() > 250 && size < 10)
		return;
	
	float splatsize = 90;
	
	if(size > 40.f)
		size = 40.f;
	
	size *= 0.75f;
	
	switch(config.video.levelOfDetail) {
		case 2:
			if(polyboom.size() > 160)
				return;
			
			splatsize = 90;
			size *= 1.f;
		break;
		case 1:
			if(polyboom.size() > 60)
				return;
			
			splatsize = 60;
			size *= 0.5f;
		break;
		default:
			if(polyboom.size() > 10)
				return;
			
			splatsize = 30;
			size *= 0.25f;
		break;
	}
	
	float py;
	if(!CheckInPoly(poss + Vec3f(0.f, -40, 0.f), &py)) {
		return;
	}
	
	if(flags & 1)
		py = poss.y;
	
	EERIEPOLY TheoricalSplat;
	TheoricalSplat.v[0].p.x = -splatsize;
	TheoricalSplat.v[0].p.y = py;
	TheoricalSplat.v[0].p.z = -splatsize;
	TheoricalSplat.v[1].p.x = -splatsize;
	TheoricalSplat.v[1].p.y = py;
	TheoricalSplat.v[1].p.z = splatsize;
	TheoricalSplat.v[2].p.x = splatsize;
	TheoricalSplat.v[2].p.y = py;
	TheoricalSplat.v[2].p.z = splatsize;
	TheoricalSplat.v[3].p.x = splatsize;
	TheoricalSplat.v[3].p.y = py;
	TheoricalSplat.v[3].p.z = -splatsize;
	TheoricalSplat.type = POLY_QUAD;
	
	Vec3f RealSplatStart(-size, py, -size);
	
	TheoricalSplat.v[0].p.x += poss.x;
	TheoricalSplat.v[0].p.z += poss.z;
	
	TheoricalSplat.v[1].p.x += poss.x;
	TheoricalSplat.v[1].p.z += poss.z;
	
	TheoricalSplat.v[2].p.x += poss.x;
	TheoricalSplat.v[2].p.z += poss.z;
	
	TheoricalSplat.v[3].p.x += poss.x;
	TheoricalSplat.v[3].p.z += poss.z;
	
	RealSplatStart.x += poss.x;
	RealSplatStart.z += poss.z;
	
	float div = 1.f / (size * 2);
	
	GameInstant now = g_gameTime.now();
	
	for(POLYBOOM & pb : polyboom) {
		pb.fastdecay = true;
	}
	
	for(auto tile : g_tiles->tilesAround(g_tiles->getTile(poss), 3)) {
		for(EERIEPOLY & polygon : tile.intersectingPolygons()) {
			
			if((flags & 2) && !(polygon.type & POLY_WATER)) {
				continue;
			}
			
			if((polygon.type & POLY_TRANS) && !(polygon.type & POLY_WATER)) {
				continue;
			}
			
			size_t nbvert = (polygon.type & POLY_QUAD) ? 4 : 3;
			
			bool oki = false;
			
			for(size_t k = 0; k < nbvert; k++) {
				
				if(PointIn2DPolyXZ(&TheoricalSplat, polygon.v[k].p.x, polygon.v[k].p.z)
				   && glm::abs(polygon.v[k].p.y - py) < 100.f) {
					oki = true;
					break;
				}
				
				if(PointIn2DPolyXZ(&TheoricalSplat, (polygon.v[k].p.x + polygon.center.x) * 0.5f,
				                                    (polygon.v[k].p.z + polygon.center.z) * 0.5f)
				   && glm::abs(polygon.v[k].p.y - py) < 100.f) {
					oki = true;
					break;
				}
				
			}
			
			if(!oki && PointIn2DPolyXZ(&TheoricalSplat, polygon.center.x, polygon.center.z)
			   && glm::abs(polygon.center.y - py) < 100.f) {
				oki = true;
			}
			
			if(oki) {
				
				if(polyboom.size() < MAX_POLYBOOM) {
					POLYBOOM pb;
					
					if(flags & 2) {
						pb.type = WaterDecal;
						
						long num = Random::get(0, 2);
						pb.tc = g_particleTextures.water_splat[num];
						
						pb.tolive = 1500ms;
					} else {
						pb.type = BloodDecal;
						
						long num = Random::get(0, 5);
						pb.tc = g_particleTextures.bloodsplat[num];
						
						pb.tolive = 400ms * size;
					}
					
					pb.fastdecay = false;
					
					pb.ep = &polygon;
					
					pb.timecreation = now;
					pb.rgb = col;
					
					for(size_t k = 0; k < nbvert; k++) {
						
						float vdiff = glm::abs(polygon.v[k].p.y - RealSplatStart.y);
						
						pb.u[k] = (polygon.v[k].p.x - RealSplatStart.x) * div;
						if(pb.u[k] < 0.5f) {
							pb.u[k] -= vdiff * div;
						} else {
							pb.u[k] += vdiff * div;
						}
						
						pb.v[k] = (polygon.v[k].p.z - RealSplatStart.z) * div;
						if(pb.v[k] < 0.5f) {
							pb.v[k] -= vdiff * div;
						} else {
							pb.v[k] += vdiff * div;
						}
						
					}
					
					pb.nbvert = short(nbvert);
					
					polyboom.push_back(pb);
				}
			}
			
		}
	}
	
}


void PolyBoomDraw() {
	
	ARX_PROFILE_FUNC();
	
	GRenderer->SetFogColor(Color::none); // TODO: not handled by RenderMaterial
	GameInstant now = g_gameTime.now();
	
	for(size_t i = 0; i < polyboom.size(); i++) {
		
		POLYBOOM & pb = polyboom[i];
		
		if(pb.fastdecay) {
			if(pb.timecreation - g_gameTime.lastFrameDuration() > 0) {
				pb.timecreation -= g_gameTime.lastFrameDuration();
			}
			if(pb.timecreation - g_gameTime.lastFrameDuration() > 0) {
				pb.timecreation -= g_gameTime.lastFrameDuration();
			}
		}
		
		GameDuration t = pb.timecreation + pb.tolive - now;
		if(t <= 0) {
			std::swap(polyboom[i], polyboom.back());
			polyboom.pop_back();
			i--;
		}
	}
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setDepthBias(8);
	mat.setLayer(RenderMaterial::Decal);
	mat.setWrapMode(TextureStage::WrapClamp);
	
	std::vector<POLYBOOM>::iterator itr = polyboom.begin();
	while (itr != polyboom.end()) {
		
		POLYBOOM & pb = *itr;
		
		arx_assume(pb.nbvert == 3 || pb.nbvert == 4);
		
		GameDuration t = pb.timecreation + pb.tolive - now;
		
		switch(pb.type) {
			
			case ScorchMarkDecal: {
				
				float tt = t / pb.tolive * 0.8f;
				ColorRGBA col = (player.m_improve ? (Color3f::red * (tt * 0.5f)) : Color3f::gray(tt)).toRGB();
				
				std::array<TexturedVertexUntransformed, 4> ltv;
				
				for(long k = 0; k < pb.nbvert; k++) {
					ltv[k].p = pb.ep->v[k].p;
					ltv[k].uv.x = pb.u[k];
					ltv[k].uv.y = pb.v[k];
					ltv[k].color = col;
				}
				
				if(player.m_improve) {
					mat.setBlendType(RenderMaterial::Additive);
				} else {
					mat.setBlendType(RenderMaterial::Subtractive);
				}
				mat.setTexture(Boom);
				
				drawTriangle(mat, ltv.data());
				if(pb.nbvert == 4) {
					drawTriangle(mat, ltv.data() + 1);
				}
				
				break;
			}
			
			case BloodDecal: {
				
				float tt = t / pb.tolive;
				float tr = std::max(1.f, tt * 2 - 0.5f);
				ColorRGBA col = Color4f(pb.rgb * tt, glm::clamp(tt * 1.5f, 0.f, 1.f)).toRGBA();
				
				std::array<TexturedVertexUntransformed, 4> ltv;
				
				for(long k = 0; k < pb.nbvert; k++) {
					ltv[k].p = pb.ep->v[k].p;
					ltv[k].uv.x = (pb.u[k] - 0.5f) * tr + 0.5f;
					ltv[k].uv.y = (pb.v[k] - 0.5f) * tr + 0.5f;
					ltv[k].color = col;
				}
				
				mat.setBlendType(RenderMaterial::Subtractive2);
				mat.setTexture(pb.tc);
				
				drawTriangle(mat, ltv.data());
				if(pb.nbvert == 4) {
					drawTriangle(mat, ltv.data() + 1);
				}
				
				break;
			}
			
			case WaterDecal: {
				
				float tt = t / pb.tolive;
				float tr = std::max(1.f, tt * 2 - 0.5f);
				float ttt = tt * 0.5f;
				ColorRGBA col = (pb.rgb * ttt).toRGB();
				
				std::array<TexturedVertexUntransformed, 4> ltv;
				
				for(long k = 0; k < pb.nbvert; k++) {
					ltv[k].p = pb.ep->v[k].p;
					ltv[k].uv.x = (pb.u[k] - 0.5f) * tr + 0.5f;
					ltv[k].uv.y = (pb.v[k] - 0.5f) * tr + 0.5f;
					ltv[k].color = col;
				}
				
				if(ltv[0].uv.x < 0.f && ltv[1].uv.x < 0.f && ltv[2].uv.x < 0.f
				   && (pb.nbvert != 4 || ltv[3].uv.x < 0.f)) {
					break;
				}
				
				if(ltv[0].uv.y < 0.f && ltv[1].uv.y < 0.f && ltv[2].uv.y < 0.f
				   && (pb.nbvert != 4 || ltv[3].uv.y < 0.f)) {
					break;
				}
				
				if(ltv[0].uv.x > 1.f && ltv[1].uv.x > 1.f && ltv[2].uv.x > 1.f
				   && (pb.nbvert != 4 || ltv[3].uv.x > 1.f)) {
					break;
				}
				
				if(ltv[0].uv.y > 1.f && ltv[1].uv.y > 1.f && ltv[2].uv.y > 1.f
				   && (pb.nbvert != 4 || ltv[3].uv.y > 1.f)) {
					break;
				}
				
				mat.setBlendType(RenderMaterial::Screen);
				mat.setTexture(pb.tc);
				
				drawTriangle(mat, ltv.data());
				if(pb.nbvert == 4) {
					drawTriangle(mat, ltv.data() + 1);
				}
				
				break;
			}
		}
		
		++itr;
	}
	
	GRenderer->SetFogColor(g_fogColor);
}
