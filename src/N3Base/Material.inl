#ifndef CLIENT_N3BASE_MATERIAL_INL
#define CLIENT_N3BASE_MATERIAL_INL

#pragma once

#include "My_3DStruct.h"

void __Material::Init(const _D3DCOLORVALUE& diffuseColor)
{
	*this        = {};

	Diffuse      = diffuseColor;
	Ambient.a    = Diffuse.a;
	Ambient.r    = Diffuse.r * 0.5f;
	Ambient.g    = Diffuse.g * 0.5f;
	Ambient.b    = Diffuse.b * 0.5f;

	dwColorOp    = D3DTOP_MODULATE;
	dwColorArg1  = D3DTA_DIFFUSE;
	dwColorArg2  = D3DTA_TEXTURE;
	nRenderFlags = RF_NOTHING;
	dwSrcBlend   = D3DBLEND_SRCALPHA;
	dwDestBlend  = D3DBLEND_INVSRCALPHA;
}

void __Material::Init() // 기본 흰색으로 만든다..
{
	D3DCOLORVALUE crDiffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	Init(crDiffuse);
}

void __Material::ColorSet(const _D3DCOLORVALUE& crDiffuse)
{
	Diffuse   = crDiffuse;
	Ambient.a = Diffuse.a;
	Ambient.r = Diffuse.r * 0.5f;
	Ambient.g = Diffuse.g * 0.5f;
	Ambient.b = Diffuse.b * 0.5f;
}

#endif // CLIENT_N3BASE_MATERIAL_INL
