/*
 *  brlyt.c
 *  
 *
 *  Created by Alex Marshall on 09/01/27.
 *  Updated by Stephen Simpson on 09/03/11.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mxml.h>

#include "general.h"
#include "types.h"
#include "brlyt.h"
#include "memory.h"
#include "xml.h"

#ifdef DEBUGBRLYT
#define dbgprintf	printf
#else
#define dbgprintf	//
#endif //DEBUGBRLYT

#define MAXIMUM_TAGS_SIZE		(0x1000)

char pic1_magic[] = "pic1";
char pan1_magic[] = "pan1";
char bnd1_magic[] = "bnd1";
char wnd1_magic[] = "wnd1";
char lyt1_magic[] = "lyt1";
char grp1_magic[] = "grp1";
char txl1_magic[] = "txl1";
char mat1_magic[] = "mat1";
char fnl1_magic[] = "fnl1";
char txt1_magic[] = "txt1";
char gre1_magic[] = "gre1";
char grs1_magic[] = "grs1";
char pae1_magic[] = "pae1";
char pas1_magic[] = "pas1";

static size_t BRLYT_fileoffset = 0;

char *materials;
int numberOfMaterials;
int lengthOfMaterials;

static int FourCCsMatch(fourcc cc1, fourcc cc2)
{
	dbgprintf("FourCCs\n");
	int ret[4];
	dbgprintf("Let's go %08x %08x\n", cc1, cc2);
	ret[0] = (cc1[0] == cc2[0]);
	dbgprintf("Got zero |%02x| |%02x| %d\n", cc1[0], cc2[0], ret[0]);
	ret[1] = (cc1[1] == cc2[1]);
	dbgprintf("Got one |%02x| |%02x| %d\n", cc1[1], cc2[1], ret[1]);
	ret[2] = (cc1[2] == cc2[2]);
	dbgprintf("Got two |%02x| |%02x| %d\n", cc1[2], cc2[2], ret[2]);
	ret[3] = (cc1[3] == cc2[3]);
	dbgprintf("Got three |%02x| |%02x| %d\n", cc1[3], cc2[3], ret[3]);
	int retval;
	if(ret[0] && ret[1] && ret[2] && ret[3])
		retval = 1;
	else
		retval = 0;
	dbgprintf("Got retval %d\n", retval);
	return retval;
}

static void BRLYT_ReadDataFromMemoryX(void* destination, void* input, size_t size)
{
	u8* out = (u8*)destination;
	u8* in = ((u8*)input) + BRLYT_fileoffset;
	memcpy(out, in, size);
}

static void BRLYT_ReadDataFromMemory(void* destination, void* input, size_t size)
{
	BRLYT_ReadDataFromMemoryX(destination, input, size);
	BRLYT_fileoffset += size;
}

float float_swap_bytes(float float1)
{
	unsigned char *float1c; float1c = (unsigned char*)&float1;

        unsigned char charTemp = 0x00;
	charTemp = float1c[0]; float1c[0] = float1c[3]; float1c[3] = charTemp;
	charTemp = 0x00;
	charTemp = float1c[1]; float1c[1] = float1c[2]; float1c[2] = charTemp;

	float *newFloat; newFloat = (float*)float1c;
	return *newFloat;
}

unsigned int bit_extract(unsigned int num, unsigned int start, unsigned int end)
{
	if (end == 100) end = start;
	//simple bitmask, figure out when awake
	//unsigned int mask = (2**(31 - start + 1) - 1) - (2**(31 - end) - 1)
	unsigned int mask;
	int first = 0;
	int firstMask = 1;
	for (first;first<31-start+1;first++)
	{
		firstMask *= 2;
	}
	firstMask -= 1;
	first = 0;
	int secondMask = 1;

	for (first;first< 31-end;first++)
	{
		secondMask *=2;
	}
	mask = firstMask = secondMask;
	int ret = (num & mask) >> (31 - end);
	//printf("%08x, %08x, %08x, %08x\n", firstMask, secondMask, mask, ret);
	return ret;
}

char* getMaterial(int offset)
{

	if (offset == 0) return materials;
	char *foo = materials + strlen(materials) + 1;

	if (offset > 1)
	{
		int n = 1;
		for (n; n<offset;n++)
		{
			foo = foo + strlen(foo) + 1;
		}
	}

	return foo;
}

int BRLYT_ReadEntries(u8* brlyt_file, size_t file_size, brlyt_header header, brlyt_entry* entries)
{
}

void BRLYT_CheckHeaderSanity(brlyt_header header, size_t filesize)
{
	if((header.magic[0] != 'R') || (header.magic[1] != 'L') || (header.magic[2] != 'Y') || (header.magic[3] != 'T')) {
		printf("BRLYT magic doesn't match! %c%c%c%c\n", header.magic[0], header.magic[1], header.magic[2], header.magic[3]);
		exit(1);
	}
	if(filesize != be32(header.filesize)) {
		printf("BRLYT filesize doesn't match!\n");
		exit(1);
	}
}

void PrintBRLYTEntry_lyt1(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_lytheader_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_lytheader_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
	printf("		Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		a: %08x\n", data.a);
	printf("		width: %f\n", float_swap_bytes(data.width));
	printf("		height: %f\n", float_swap_bytes(data.height));
#else
	printf("type=\"%c%c%c%c\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		<a>%08x</a>\n", data.a);
	printf("		<size>\n");
	printf("			<width>%f</width>\n", float_swap_bytes(data.width));
	printf("			<height>%f</height>\n", float_swap_bytes(data.height));
	printf("		</size>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_grp1(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_group_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_group_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
	printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		Name: %s\n", data.name);
	printf("		Number of subs: %08x\n", be16(data.numsubs));
	printf("		unk: %08x\n", be16(data.unk));
#else
	printf("type=\"%c%c%c%c\" name=\"%s\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3], data.name);
	printf("		<subs>\n");
#endif //OLD_BRLYT_OUTSTYLE

	int offset;
	offset = 20;
	int n = 0;
	for (n;n<be16(data.numsubs);n++)
	{
		char sub[16];
		BRLYT_ReadDataFromMemory(sub, brlyt_file, sizeof(sub));
#ifdef OLD_BRLYT_OUTSTYLE
		printf("                sub: %s\n", sub);
#else
		printf("		<sub>%s</sub>\n", sub);
#endif //OLD_BRLYT_OUTSTYLE
		offset += 16;
	}
#ifndef OLD_BRLYT_OUTSTYLE
	printf("		</subs>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_txl1(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_numoffs_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_numoffs_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
	printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("                num: %08x\n", be16(data.num));
	printf("                offs: %08x\n", be16(data.offs));
#else
	printf("type=\"%c%c%c%c\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		<entries>\n");
#endif //OLD_BRLYT_OUTSTYLE
	int pos = 4;
	pos += data.offs;
	int bpos = pos;
	int n = 0;
	for (n;n<be16(data.num);n++)
	{
                brlyt_offsunk_chunk data2;
                BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_offsunk_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                printf("                offset: %08x\n", be32(data2.offset));
                printf("                unk: %08x\n", be32(data2.unk));
#else
#endif //OLD_BRLYT_OUTSTYLE
                int tempLocation = BRLYT_fileoffset;
                BRLYT_fileoffset = entry.data_location + bpos + be32(data2.offset);
		int toRead = (be32(entry.length) + entry.data_location - 8) - BRLYT_fileoffset;
		char nameRead[toRead];
                BRLYT_ReadDataFromMemory(nameRead, brlyt_file, sizeof(nameRead));
                //char nameRead[toRead] the name of the tpls null terminated between
		char tpl = 0;
		char *ending = memchr(nameRead, tpl, toRead);
		int end = ending - nameRead;
		char name[end+1];
		memcpy(name, nameRead, sizeof(name));
#ifdef OLD_BRLYT_OUTSTYLE
		printf("                name: %s\n", name);
#else
		printf("			<name>%s</name>\n", name);
#endif //OLD_BRLYT_OUTSTYLE
                BRLYT_fileoffset = tempLocation;
		int oldsize = sizeof(name);
		dbgprintf("size of materials before: %08x\n", sizeof(materials));
		int newSize = lengthOfMaterials+sizeof(name);
		materials = realloc(materials, newSize);
		numberOfMaterials += 1;
		memcpy(materials+lengthOfMaterials, name, sizeof(name));
		dbgprintf("mats: %s\tnamsize: %08x\tmatsize: %08x\tnewsize: %08x\n", materials+lengthOfMaterials, sizeof(name), lengthOfMaterials, newSize);
		lengthOfMaterials = newSize;
	}
#ifndef OLD_BRLYT_OUTSTYLE
	printf("		</entries>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_fnl1(brlyt_entry entry, u8* brlyt_file)
{
        brlyt_numoffs_chunk data;
        BRLYT_fileoffset = entry.data_location;
        BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_numoffs_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
	printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
        printf("                num: %08x\n", be16(data.num));
        printf("                offs: %08x\n", be16(data.offs));
#else
	printf("type=\"%c%c%c%c\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		<entries>\n");
#endif //OLD_BRLYT_OUTSTYLE
        int pos = 4;
        pos += data.offs;
        int bpos = pos;
        int n = 0;
	for (n;n<be16(data.num);n++)
        {
                brlyt_offsunk_chunk data2;
                BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_offsunk_chunk));
                //int data2.offset              //int data2.unk
#ifdef OLD_BRLYT_OUTSTYLE
                printf("                offset: %08x\n", be32(data2.offset));
                printf("                unk: %08x\n", be32(data2.unk));
#else
#endif //OLD_BRLYT_OUTSTYLE
                int tempLocation = BRLYT_fileoffset;
                BRLYT_fileoffset = entry.data_location + bpos + be32(data2.offset);
                int toRead = (be32(entry.length) + entry.data_location - 8) - BRLYT_fileoffset;
                char nameRead[toRead];
                BRLYT_ReadDataFromMemory(nameRead, brlyt_file, sizeof(nameRead));
                //char nameRead[toRead] the name of the tpls null terminated between
                char tpl = 0;
                char *ending = memchr(nameRead, tpl, toRead);
                int end = ending - nameRead;
                char name[end+1];
                memcpy(name, nameRead, sizeof(name));
#ifdef OLD_BRLYT_OUTSTYLE
                printf("                name: %s\n", name);
#else
		printf("			<name>%s</name>\n", name);
#endif //OLD_BRLYT_OUTSTYLE
                BRLYT_fileoffset = tempLocation;
        }
#ifndef OLD_BRLYT_OUTSTYLE
	printf("		</entries>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_pan1(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_pane_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_pane_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
        printf("                flag1: %08x\n", data.flag1);
        printf("                flag2: %08x\n", data.flag2);
        printf("                alpha: %08x\n", data.alpha);
        printf("                alpha2: %08x\n", data.alpha2);
        printf("                name: %s\n", data.name);
        printf("                x: %f\n", be32(data.x));
        printf("                y: %f\n", be32(data.y));
        printf("                z: %f\n", be32(data.z));
        printf("                flip_x: %f\n", float_swap_bytes(data.flip_x));
        printf("                flip_y: %f\n", float_swap_bytes(data.flip_y));
        printf("                angle: %f\n", float_swap_bytes(data.angle));
        printf("                xmag: %f\n", float_swap_bytes(data.xmag));
        printf("                ymag: %f\n", float_swap_bytes(data.ymag));
        printf("                width: %f\n", float_swap_bytes(data.width));
        printf("                height: %f\n", float_swap_bytes(data.height));
#else
	printf("type=\"%c%c%c%c\" name=\"%s\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3], data.name);
	printf("		<flags>%08x-%08x</flags>\n", data.flag1, data.flag2);
	printf("		<alpha>%08x-%08x</alpha>\n", data.alpha, data.alpha2);
	printf("		<coords>\n");
	printf("			<x>%f</x>\n", be32(data.x));
	printf("			<y>%f</y>\n", be32(data.y));
	printf("			<z>%f</z>\n", be32(data.z));
	printf("		</coords>\n");
	printf("		<flip>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.flip_x));
	printf("			<y>%f</y>\n", float_swap_bytes(data.flip_y));
	printf("		</flip>\n");
	printf("		<rotate>%f</rotate>\n", float_swap_bytes(data.angle));
	printf("		<zoom>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.xmag));
	printf("			<y>%f</y>\n", float_swap_bytes(data.ymag));
	printf("		</zoom>\n");
	printf("		<size>\n");
	printf("			<width>%f</width>\n", float_swap_bytes(data.width));
	printf("			<height>%f</height>\n", float_swap_bytes(data.height));
	printf("		</size>\n");
	printf("	</tag>\n");	
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_wnd1(brlyt_entry entry, u8* brlyt_file)
{
        brlyt_pane_chunk data;
        BRLYT_fileoffset = entry.data_location;
        BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_pane_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
        printf("                flag1: %08x\n", data.flag1);
        printf("                flag2: %08x\n", data.flag2);
        printf("                alpha: %08x\n", data.alpha);
        printf("                alpha2: %08x\n", data.alpha2);
        printf("                name: %s\n", data.name);
        printf("                x: %f\n", be32(data.x));
        printf("                y: %f\n", be32(data.y));
        printf("                z: %f\n", be32(data.z));
        printf("                flip_x: %f\n", float_swap_bytes(data.flip_x));
        printf("                flip_y: %f\n", float_swap_bytes(data.flip_y));
        printf("                angle: %f\n", float_swap_bytes(data.angle));
        printf("                xmag: %f\n", float_swap_bytes(data.xmag));
        printf("                ymag: %f\n", float_swap_bytes(data.ymag));
        printf("                width: %f\n", float_swap_bytes(data.width));
        printf("                height: %f\n", float_swap_bytes(data.height));
#else
	printf("type=\"%c%c%c%c\" name=\"%s\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3], data.name);
	printf("		<flags>%08x-%08x</flags>\n", data.flag1, data.flag2);
	printf("		<alpha>%08x-%08x</alpha>\n", data.alpha, data.alpha2);
	printf("		<coords>\n");
	printf("			<x>%f</x>\n", be32(data.x));
	printf("			<y>%f</y>\n", be32(data.y));
	printf("			<z>%f</z>\n", be32(data.z));
	printf("		</coords>\n");
	printf("		<flip>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.flip_x));
	printf("			<y>%f</y>\n", float_swap_bytes(data.flip_y));
	printf("		</flip>\n");
	printf("		<rotate>%f</rotate>\n", float_swap_bytes(data.angle));
	printf("		<zoom>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.xmag));
	printf("			<y>%f</y>\n", float_swap_bytes(data.ymag));
	printf("		</zoom>\n");
	printf("		<size>\n");
	printf("			<width>%f</width>\n", float_swap_bytes(data.width));
	printf("			<height>%f</height>\n", float_swap_bytes(data.height));
	printf("		</size>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_bnd1(brlyt_entry entry, u8* brlyt_file)
{
	dbgprintf("entry length: %08x\n", be32(entry.length));
	int w;
	for (w=0;w<be32(entry.length);w++) dbgprintf("byte %x: %02x\n", w, brlyt_file[w]);

        brlyt_pane_chunk data;
        BRLYT_fileoffset = entry.data_location;
        BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_pane_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
        printf("                flag1: %08x\n", data.flag1);
        printf("                flag2: %08x\n", data.flag2);
        printf("                alpha: %08x\n", data.alpha);
        printf("                alpha2: %08x\n", data.alpha2);
        printf("                name: %s\n", data.name);
        printf("                x: %f\n", be32(data.x));
        printf("                y: %f\n", be32(data.y));
        printf("                z: %f\n", be32(data.z));
        printf("                flip_x: %f\n", float_swap_bytes(data.flip_x));
        printf("                flip_y: %f\n", float_swap_bytes(data.flip_y));
        printf("                angle: %f\n", float_swap_bytes(data.angle));
        printf("                xmag: %f\n", float_swap_bytes(data.xmag));
        printf("                ymag: %f\n", float_swap_bytes(data.ymag));
        printf("                width: %f\n", float_swap_bytes(data.width));
        printf("                height: %f\n", float_swap_bytes(data.height));
#else
	printf("type=\"%c%c%c%c\" name=\"%s\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3], data.name);
	printf("		<flags>%08x-%08x</flags>\n", data.flag1, data.flag2);
	printf("		<alpha>%08x-%08x</alpha>\n", data.alpha, data.alpha2);
	printf("		<coords>\n");
	printf("			<x>%f</x>\n", be32(data.x));
	printf("			<y>%f</y>\n", be32(data.y));
	printf("			<z>%f</z>\n", be32(data.z));
	printf("		</coords>\n");
	printf("		<flip>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.flip_x));
	printf("			<y>%f</y>\n", float_swap_bytes(data.flip_y));
	printf("		</flip>\n");
	printf("		<rotate>%f</rotate>\n", float_swap_bytes(data.angle));
	printf("		<zoom>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.xmag));
	printf("			<y>%f</y>\n", float_swap_bytes(data.ymag));
	printf("		</zoom>\n");
	printf("		<size>\n");
	printf("			<width>%f</width>\n", float_swap_bytes(data.width));
	printf("			<height>%f</height>\n", float_swap_bytes(data.height));
	printf("		</size>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_pic1(brlyt_entry entry, u8* brlyt_file)
{
        brlyt_pane_chunk data;
        BRLYT_fileoffset = entry.data_location;
        BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_pane_chunk));
	brlyt_pic_chunk data2;
	BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_pic_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
        printf("                flag1: %08x\n", data.flag1);
	printf("                flag2: %08x\n", data.flag2);
	printf("                alpha: %08x\n", data.alpha);
	printf("                alpha2: %08x\n", data.alpha2);
	printf("                name: %s\n", data.name);
	printf("                x: %f\n", be32(data.x));
	printf("                y: %f\n", be32(data.y));
	printf("                z: %f\n", be32(data.z));
	printf("                flip_x: %f\n", float_swap_bytes(data.flip_x));
	printf("                flip_y: %f\n", float_swap_bytes(data.flip_y));
	printf("                angle: %f\n", float_swap_bytes(data.angle));
	printf("                xmag: %f\n", float_swap_bytes(data.xmag));
	printf("                ymag: %f\n", float_swap_bytes(data.ymag));
	printf("                width: %f\n", float_swap_bytes(data.width));
	printf("                height: %f\n", float_swap_bytes(data.height));
#else
	printf("type=\"%c%c%c%c\" name=\"%s\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3], data.name);
	printf("		<flags>%08x-%08x</flags>\n", data.flag1, data.flag2);
	printf("		<alpha>%08x-%08x</alpha>\n", data.alpha, data.alpha2);
	printf("		<coords>\n");
	printf("			<x>%f</x>\n", be32(data.x));
	printf("			<y>%f</y>\n", be32(data.y));
	printf("			<z>%f</z>\n", be32(data.z));
	printf("		</coords>\n");
	printf("		<flip>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.flip_x));
	printf("			<y>%f</y>\n", float_swap_bytes(data.flip_y));
	printf("		</flip>\n");
	printf("		<rotate>%f</rotate>\n", float_swap_bytes(data.angle));
	printf("		<zoom>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.xmag));
	printf("			<y>%f</y>\n", float_swap_bytes(data.ymag));
	printf("		</zoom>\n");
	printf("		<size>\n");
	printf("			<width>%f</width>\n", float_swap_bytes(data.width));
	printf("			<height>%f</height>\n", float_swap_bytes(data.height));
	printf("		</size>\n");
#endif //OLD_BRLYT_OUTSTYLE
#ifdef OLD_BRLYT_OUTSTYLE
	printf("                vtx_colors: %u,%u,%u,%u\n", be32(data2.vtx_colors[0]), be32(data2.vtx_colors[1]), be32(data2.vtx_colors[2]), be32(data2.vtx_colors[3]));
	printf("                mat_off: %08x\n", be16(data2.mat_off));
	printf("                num_texcoords: %08x\n", data2.num_texcoords);
	printf("                padding: %08x\n", data2.padding);
#else
	printf("		<colors>\n");
	printf("			<vtx>0x%08X</vtx>\n", be32(data2.vtx_colors[0]));
	printf("			<vtx>0x%08X</vtx>\n", be32(data2.vtx_colors[1]));
	printf("			<vtx>0x%08X</vtx>\n", be32(data2.vtx_colors[2]));
	printf("			<vtx>0x%08X</vtx>\n", be32(data2.vtx_colors[3]));
	printf("		</colors>\n");
	printf("		<coordinates>\n");
#endif //OLD_BRLYT_OUTSTYLE
	int n = 0;
	for (n;n<data2.num_texcoords;n++)
	{
		float texcoords[8];		// I think that's what that means 
		BRLYT_ReadDataFromMemory(texcoords, brlyt_file, sizeof(texcoords));
#ifdef OLD_BRLYT_OUTSTYLE
        	printf("                tex coords: %f,%f,%f,%f,%f,%f,%f,%f\n", float_swap_bytes(texcoords[0]), float_swap_bytes(texcoords[1]), float_swap_bytes(texcoords[2]), float_swap_bytes(texcoords[3]), float_swap_bytes(texcoords[4]), float_swap_bytes(texcoords[5]), float_swap_bytes(texcoords[6]), float_swap_bytes(texcoords[7]));
#else
		int i;
		printf("			<set>\n");
		for(i = 0; i < 8; i++)
			printf("				<coord>%f</coord>\n", float_swap_bytes(texcoords[i]));
		printf("			</set>\n");
#endif //OLD_BRLYT_OUTSTYLE
	}
#ifndef OLD_BRLYT_OUTSTYLE
	printf("		</coordinates>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_txt1(brlyt_entry entry, u8* brlyt_file)
{
        brlyt_pane_chunk data;
        BRLYT_fileoffset = entry.data_location;
        BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_pane_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
        printf("                flag1: %08x\n", data.flag1);
        printf("                flag2: %08x\n", data.flag2);
        printf("                alpha: %08x\n", data.alpha);
        printf("                alpha2: %08x\n", data.alpha2);
        printf("                name: %s\n", data.name);
        printf("                x: %f\n", be32(data.x));
        printf("                y: %f\n", be32(data.y));
        printf("                z: %f\n", be32(data.z));
        printf("                flip_x: %f\n", float_swap_bytes(data.flip_x));
        printf("                flip_y: %f\n", float_swap_bytes(data.flip_y));
        printf("                angle: %f\n", float_swap_bytes(data.angle));
        printf("                xmag: %f\n", float_swap_bytes(data.xmag));
        printf("                ymag: %f\n", float_swap_bytes(data.ymag));
        printf("                width: %f\n", float_swap_bytes(data.width));
        printf("                height: %f\n", float_swap_bytes(data.height));
#else
	printf("type=\"%c%c%c%c\" name=\"%s\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3], data.name);
	printf("		<flags>%08x-%08x</flags>\n", data.flag1, data.flag2);
	printf("		<alpha>%08x-%08x</alpha>\n", data.alpha, data.alpha2);
	printf("		<coords>\n");
	printf("			<x>%f</x>\n", be32(data.x));
	printf("			<y>%f</y>\n", be32(data.y));
	printf("			<z>%f</z>\n", be32(data.z));
	printf("		</coords>\n");
	printf("		<flip>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.flip_x));
	printf("			<y>%f</y>\n", float_swap_bytes(data.flip_y));
	printf("		</flip>\n");
	printf("		<rotate>%f</rotate>\n", float_swap_bytes(data.angle));
	printf("		<zoom>\n");
	printf("			<x>%f</x>\n", float_swap_bytes(data.xmag));
	printf("			<y>%f</y>\n", float_swap_bytes(data.ymag));
	printf("		</zoom>\n");
	printf("		<size>\n");
	printf("			<width>%f</width>\n", float_swap_bytes(data.width));
	printf("			<height>%f</height>\n", float_swap_bytes(data.height));
	printf("		</size>\n");
#endif //OLD_BRLYT_OUTSTYLE
	brlyt_text_chunk data2;
	BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_text_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
	printf("                len1: %08x\n", be16(data2.len1));
	printf("                len2: %08x\n", be16(data2.len2));
	printf("                mat_off: %08x\n", be16(data2.mat_off));
	printf("                font_idx: %08x\n", be16(data2.font_idx));
	printf("                unk4: %08x\n", data2.unk4);
	printf("                pad[3]: %08x\n", data2.pad[0], data2.pad[1], data2.pad[2]);
	printf("                name_offs: %08x\n", be32(data2.name_offs));
	printf("                color1: %08x\n", be32(data2.color1));
	printf("                color2: %08x\n", be32(data2.color2));
        printf("                font_size_x: %f\n", float_swap_bytes(data2.font_size_x));
        printf("                font_size_y: %f\n", float_swap_bytes(data2.font_size_y));
        printf("                char_space: %f\n", float_swap_bytes(data2.char_space));
        printf("                line_space: %f\n", float_swap_bytes(data2.line_space));
#else
	printf("		<length>%04x-%04x</length>\n", be16(data2.len1), be16(data2.len2));
	printf("		<font index=\"%d\">\n", be16(data2.font_idx));
	printf("			<xsize>%f</xsize>\n", float_swap_bytes(data2.font_size_x));
	printf("			<ysize>%f</ysize>\n", float_swap_bytes(data2.font_size_y));
	printf("			<xsize>%f</xsize>\n", float_swap_bytes(data2.font_size_x));
	printf("			<charsize>%f</charsize>\n", float_swap_bytes(data2.char_space));
	printf("			<linesize>%f</linesize>\n", float_swap_bytes(data2.line_space));
	printf("		</font>\n");
	printf("		<color>%08x-%08x</color>\n", be32(data2.color1), be32(data2.color2));
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_mat1(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_numoffs_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_numoffs_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
	printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("                num: %08x\n", be16(data.num));
	printf("                offs: %08x\n", be16(data.offs));
#else
	printf("type=\"%c%c%c%c\" count=\"%d\">\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3], be16(data.num));
	printf("		<entries>\n");
#endif //OLD_BRLYT_OUTSTYLE
	int n = 0;
	for (n;n<be16(data.num);n++)
	{
		int offset;
		BRLYT_ReadDataFromMemory(&offset, brlyt_file, sizeof(offset));
		int tempDataLocation = BRLYT_fileoffset;
		BRLYT_fileoffset = entry.data_location + be32(offset) - 8;
		brlyt_material_chunk data3;
		BRLYT_ReadDataFromMemory(&data3, brlyt_file, sizeof(brlyt_material_chunk));
		
		//more junk to do with bit masks and flags
		//mat_texref = get_array(chunk, mpos, bit_extract(data3.flags, 28,31), 4, 'texref');
		unsigned int flaggs = be32(data3.flags);
#ifdef OLD_BRLYT_OUTSTYLE
		printf("                offset: %08x\n", be32(offset));
		printf("                name: %s\n", data3.name);
		printf("                tev_color: %#x,%#x,%#x,%#x\n", be16(data3.tev_color[0]), be16(data3.tev_color[1]), be16(data3.tev_color[2]), be16(data3.tev_color[3]));
		printf("                unk_color: %#x,%#x,%#x,%#x\n", be16(data3.unk_color[0]), be16(data3.unk_color[1]), be16(data3.unk_color[2]), be16(data3.unk_color[3]));
		printf("                unk_color_2: %#x,%#x,%#x,%#x\n", be16(data3.unk_color_2[0]), be16(data3.unk_color_2[1]), be16(data3.unk_color_2[2]), be16(data3.unk_color_2[3]));
		printf("                tev_kcolor: %#x,%#x,%#x,%#x\n", be32(data3.tev_kcolor[0]), be32(data3.tev_kcolor[1]), be32(data3.tev_kcolor[2]), be32(data3.tev_kcolor[3]));
		printf("                flags: %08x\n", be32(data3.flags));
		printf("                bitmask: %08x\n", bit_extract(flaggs, 28, 31));
#else
#endif //OLD_BRLYT_OUTSTYLE
		int n = 0;
		for (n;n<bit_extract(flaggs, 28,31);n++)
		{
			brlyt_texref_chunk data4;
			BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_texref_chunk));
			int tplOffset = be16(data4.tex_offs);
#ifdef OLD_BRLYT_OUTSTYLE
			printf("                texoffs: %08x\n", be16(data4.tex_offs));
			printf("                wrap_s: %08x\n", data4.wrap_s);
			printf("                wrap_t: %08x\n", data4.wrap_t);
			printf("                name: %s\n", getMaterial(tplOffset));
#else
#endif //OLD_BRLYT_OUTSTYLE
		}

//		# 0x14 * flags[24-27], followed by
                n = 0;
                for (n;n<bit_extract(flaggs, 24,27);n++)
                {
                        brlyt_ua2_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_ua2_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua2: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# 4*flags[20-23], followed by
                n = 0;
                for (n;n<bit_extract(flaggs, 20,23);n++)
                {
                        brlyt_4b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_4b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua3: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# Changing ua3 things
		//# 1st --> disappears.
		//# 2nd --> no visible effect.
		//# 3rd --> disappears.
		//# 4th --> no visible effect.

		//# 4 * flags[6]
                n = 0;
                for (n;n<bit_extract(flaggs, 6,100);n++)
                {
                        brlyt_4b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_4b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua4: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# 4 * flags[4]
                n = 0;
                for (n;n<bit_extract(flaggs, 4,100);n++)
                {
                        brlyt_4b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_4b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua5: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# 4 * flags[19]
                n = 0;
                for (n;n<bit_extract(flaggs, 19,100);n++)
                {
                        brlyt_4b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_4b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua6: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
                n = 0;
                for (n;n<bit_extract(flaggs, 17,18);n++)
                {
                        brlyt_ua7_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_ua7_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua7 a: %08x\n", be32(data4.a));
			printf("                ua7 b: %08x\n", be32(data4.b));
			printf("                ua7 c: %f\n", float_swap_bytes(data4.c));
			printf("                ua7 d: %08x\n", be32(data4.d));
			printf("                ua7 e: %08x\n", be32(data4.e));
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# 4 * flags[14-16]
                n = 0;
                for (n;n<bit_extract(flaggs, 14,16);n++)
                {
                        brlyt_4b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_4b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua8: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# 0x10 * flags[9-13]
                n = 0;
                for (n;n<bit_extract(flaggs, 9,13);n++)
                {
                        brlyt_10b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_10b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                ua8: %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3], data4.unk[4], data4.unk[5], data4.unk[6], data4.unk[7], data4.unk[8], data4.unk[9], data4.unk[10], data4.unk[11], data4.unk[12], data4.unk[13], data4.unk[14], data4.unk[15]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# 4 * flags[8], these are bytes btw
                n = 0;
                for (n;n<bit_extract(flaggs, 20,23);n++)
                {
                        brlyt_4b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_4b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                uaa: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
		//# 4 * flags[7]
                n = 0;
                for (n;n<bit_extract(flaggs, 20,23);n++)
                {
                        brlyt_4b_chunk data4;
                        //get_opt(chunk, pos, True, item_size, item_type);
                        BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_4b_chunk));
#ifdef OLD_BRLYT_OUTSTYLE
                        printf("                uab: %08x, %08x, %08x, %08x\n", data4.unk[0], data4.unk[1], data4.unk[2], data4.unk[3]);
#else
#endif //OLD_BRLYT_OUTSTYLE
                        //pos += item_size;
                }
//		if n < vars['num'] - 1
//		{
//			next_offset, = struct.unpack('>I', chunk[pos+4:pos+8])
//			if next_offset - 8 != mpos:
//				mat['~_insane'] = next_offset - 8 - mpos //# Extra shit we di        dn't parse :(
//		}
// 		mat['unk_bit_5'] = bit_extract(flags, 5)
//		mat['unk_bits_0_3'] = bit_extract(flags, 0, 3) //# Overwritten by stu        ff
//		vars['materials'].append(mat)
//		pos += 4

		BRLYT_fileoffset = tempDataLocation;		
	}
#ifndef OLD_BRLYT_OUTSTYLE
	printf("		</entries>\n");
	printf("	</tag>\n");
#endif //OLD_BRLYT_OUTSTYLE
}

void PrintBRLYTEntry_gre1(brlyt_entry entry, u8* brlyt_file)
{
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#else
	printf("type=\"%c%c%c%c\" />\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#endif //OLD_BRLYT_OUTSTYLE
	//group end info
}

void PrintBRLYTEntry_grs1(brlyt_entry entry, u8* brlyt_file)
{
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#else
	printf("type=\"%c%c%c%c\" />\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#endif //OLD_BRLYT_OUTSTYLE
        //group start info
}

void PrintBRLYTEntry_pae1(brlyt_entry entry, u8* brlyt_file)
{
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#else
	printf("type=\"%c%c%c%c\" />\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#endif //OLD_BRLYT_OUTSTYLE
        //panel end info
}

void PrintBRLYTEntry_pas1(brlyt_entry entry, u8* brlyt_file)
{
#ifdef OLD_BRLYT_OUTSTYLE
        printf("                Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#else
	printf("type=\"%c%c%c%c\" />\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
#endif //OLD_BRLYT_OUTSTYLE
        //panel start info
}

void PrintBRLYTEntries(brlyt_entry *entries, int entrycnt, u8* brlyt_file)
{
	dbgprintf("Printing BRLYT Entries");
	int i;
	
	for(i = 0; i < entrycnt; i++) {
#ifdef OLD_BRLYT_OUTSTYLE
		printf("\n	Index %d (@%08x):\n", i, entries[i].data_location - 8);
#else
		printf("	<tag ");
#endif //OLD_BRLYT_OUTSTYLE
		if((FourCCsMatch(entries[i].magic, pan1_magic) == 1)) {
			dbgprintf("pan1\n");
			PrintBRLYTEntry_pan1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, txt1_magic) == 1)) {
			dbgprintf("txt1\n");
			PrintBRLYTEntry_txt1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, pic1_magic) == 1)) {
			dbgprintf("pic1\n");
			PrintBRLYTEntry_pic1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, wnd1_magic) == 1)) {
			dbgprintf("wnd1\n");
			PrintBRLYTEntry_wnd1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, bnd1_magic) == 1)) {
			dbgprintf("bnd1\n");
			PrintBRLYTEntry_bnd1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, lyt1_magic) == 1)) {
			dbgprintf("lyt1\n");
			PrintBRLYTEntry_lyt1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, grp1_magic) == 1)) {
			dbgprintf("grp1\n");
			PrintBRLYTEntry_grp1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, txl1_magic) == 1)) {
			dbgprintf("txl1\n");
			PrintBRLYTEntry_txl1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, fnl1_magic) == 1)) {
			dbgprintf("img\n");
			PrintBRLYTEntry_fnl1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, mat1_magic) == 1)) {
			dbgprintf("mat1\n");
			PrintBRLYTEntry_mat1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, gre1_magic) == 1)) {
			dbgprintf("gre1\n");
			PrintBRLYTEntry_gre1(entries[i], brlyt_file);
                }else if((FourCCsMatch(entries[i].magic, grs1_magic) == 1)) {
                        dbgprintf("grs1\n");
                        PrintBRLYTEntry_grs1(entries[i], brlyt_file);
                }else if((FourCCsMatch(entries[i].magic, pae1_magic) == 1)) {
                        dbgprintf("pae1\n");
                        PrintBRLYTEntry_pae1(entries[i], brlyt_file);
                }else if((FourCCsMatch(entries[i].magic, pas1_magic) == 1)) {
                        dbgprintf("pas1\n");
                        PrintBRLYTEntry_pas1(entries[i], brlyt_file);
		}else{
#ifdef OLD_BRLYT_OUTSTYLE
			printf("		Unknown tag (%c%c%c%c)!\n",entries[i].magic[0],entries[i].magic[1],entries[i].magic[2],entries[i].magic[3]);
#else
			printf("/>\n");
#endif //OLD_BRLYT_OUTSTYLE
		}
	}
}

void parse_brlyt(char *filename)
{
	materials = (char*)malloc(12);
	numberOfMaterials = 0;
	lengthOfMaterials = 0;
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("Error! Couldn't open %s!\n", filename);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	dbgprintf("Filesize is %d\n", file_size);
	u8* brlyt_file = (u8*)malloc(file_size);
	dbgprintf("brlyt_file allocated\n");
	fseek(fp, 0, SEEK_SET);
	fread(brlyt_file, file_size, 1, fp);
	dbgprintf("brlyt_file read to.\n");
	BRLYT_fileoffset = 0;
	brlyt_header header;
	BRLYT_ReadDataFromMemory(&header, brlyt_file, sizeof(brlyt_header));
	BRLYT_CheckHeaderSanity(header, file_size);
	brlyt_entry *entries;
	BRLYT_fileoffset = be16(header.lyt_offset);
	brlyt_entry_header tempentry;
	int i;
	dbgprintf("curr %08x max %08x\n", BRLYT_fileoffset, file_size);
	for(i = 0; BRLYT_fileoffset < file_size; i++) {
		BRLYT_ReadDataFromMemoryX(&tempentry, brlyt_file, sizeof(brlyt_entry_header));
		BRLYT_fileoffset += be32(tempentry.length);
		dbgprintf("curr %08x max %08x\n", BRLYT_fileoffset, file_size);
	}
	int entrycount = i;
	entries = (brlyt_entry*)calloc(entrycount, sizeof(brlyt_entry));
	dbgprintf("%08x\n", entries);
	if(entries == NULL) {
		printf("Couldn't allocate for entries!\n");
		exit(1);
	}
	BRLYT_fileoffset = be16(header.lyt_offset);
	for(i = 0; i < entrycount; i++) {
		dbgprintf("&(entries[i]) = %08x\n", &(entries[i]));
		BRLYT_ReadDataFromMemoryX(&tempentry, brlyt_file, sizeof(brlyt_entry_header));
		memcpy(entries[i].magic, tempentry.magic, 4);
		entries[i].length = tempentry.length;
		entries[i].data_location = BRLYT_fileoffset + sizeof(brlyt_entry_header);
		BRLYT_fileoffset += be32(tempentry.length);
	}	
//	int entrycnt = BRLYT_ReadEntries(brlyt_file, file_size, header, entries);
	dbgprintf("%08x\n", entries);
#ifdef OLD_BRLYT_OUTSTYLE
	printf("Parsed BRLYT! Information:\n");
	printf("Main header:\n");
	printf("	Magic: %c%c%c%c\n", header.magic[0], header.magic[1], header.magic[2], header.magic[3]);
	printf("	Unk1: %08x\n", be32(header.unk1));
	printf("	Filesize: %lu\n", be32(header.filesize));
	printf("		%s real file size!\n", be32(header.filesize) == file_size ? "Matches" : "Does not match");
	printf("	Offset to lyt1: %04x\n", be16(header.lyt_offset));
	printf("	Unk2: %04x\n", be16(header.unk2));
	printf("\nBRLYT Entries:");
#else
	printf("<?xml version=\"1.0\"?>\n" \
	       "<xmlyt>\n");
#endif //OLD_BRLYT_OUTSTYLE
	PrintBRLYTEntries(entries, entrycount, brlyt_file);
#ifndef OLD_BRLYT_OUTSTYLE
	printf("</xmlyt>\n");
#endif //OLD_BRLYT_OUTSTYLE

}

/*
u32 create_entries_from_xml(mxml_node_t *tree, mxml_node_t *node, brlan_entry *entr, tag_header* head, u8** tagblob, u32* blobsize)
{

	tag_entry* entry = NULL;
	tag_entryinfo* entryinfo = NULL;
	tag_data** data = NULL;
	mxml_node_t *tempnode = NULL;
	mxml_node_t *subnode = NULL;
	mxml_node_t *subsubnode = NULL;
	char temp[256];
	char temp2[256];
	char temp3[15][24];
	int i, x;

	for(i = 0; i < 16; i++)
		memset(temp3[i], 0, 24);
	for(x = 0; x < 16; x++)
		for(i = 0; i < strlen(tag_types_list[x]); i++)
			temp3[x][i] = toupper(tag_types_list[x][i]);
	head->entry_count = 0;
	subnode = node;
	for (x = 0, subnode = mxmlFindElement(subnode, node, "entry", NULL, NULL, MXML_DESCEND); subnode != NULL; subnode = mxmlFindElement(subnode, node, "entry", NULL, NULL, MXML_DESCEND), x++) {
		head->entry_count++;
		entry = realloc(entry, sizeof(tag_entry) * head->entry_count);
		entryinfo = realloc(entryinfo, sizeof(tag_entryinfo) * head->entry_count);
		if(data == NULL)
			data = (tag_data**)malloc(sizeof(tag_data*) * head->entry_count);
		else
			data = (tag_data**)realloc(data, sizeof(tag_data*) * head->entry_count);
		data[x] = NULL;
		memset(temp, 0, 256);
		memset(temp2, 0, 256);
		if(mxmlElementGetAttr(subnode, "type") != NULL)
			strcpy(temp, mxmlElementGetAttr(subnode, "type"));
		else{
			printf("No type attribute found!\nSkipping this entry!\n");
			head->entry_count--;
			continue;
		}
		for(i = 0; i < strlen(temp); i++)
			temp2[i] = toupper(temp[i]);
		for(i = 0; (i < 16) && (strcmp(temp3[i - 1], temp2) != 0); i++);
		if(i == 16)
			i = atoi(temp2);
		else
			i--;
		entry[x].offset = 0;
		entryinfo[x].type = i;
		entryinfo[x].unk1 = 0x0200;
		entryinfo[x].pad1 = 0x0000;
		entryinfo[x].unk2 = 0x0000000C;
		entryinfo[x].coord_count = 0;
		subsubnode = subnode;
		for (i = 0, subsubnode = mxmlFindElement(subsubnode, subnode, "triplet", NULL, NULL, MXML_DESCEND); subsubnode != NULL; subsubnode = mxmlFindElement(subsubnode, subnode, "triplet", NULL, NULL, MXML_DESCEND), i++) {
			entryinfo[x].coord_count++;
			data[x] = realloc(data[x], sizeof(tag_data) * entryinfo[x].coord_count);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "frame", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"frame\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part1)) = atof(temp);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "value", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"value\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part2)) = atof(temp);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "blend", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"blend\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part3)) = atof(temp);
		}
	}
	FILE* fp = fopen("temp.blan", "wb+");
	if(fp == NULL) {
		printf("Couldn't open temporary temp.blan file\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_SET);
	entr->anim_header_len = 0;
	WriteBRLANEntry(entr, fp);
	WriteBRLANTagHeader(head, fp);
	u32 entryloc = ftell(fp);
	WriteBRLANTagEntries(entry, head->entry_count, fp);
	u32* entryinfolocs = (u32*)calloc(head->entry_count, sizeof(u32));
	for(x = 0; x < head->entry_count; x++) {
		entryinfolocs[x] = ftell(fp);
		entry[x].offset = entryinfolocs[x] - sizeof(brlan_entry);
		WriteBRLANTagEntryinfos(entryinfo[x], fp);
		WriteBRLANTagData(data[x], entryinfo[x].coord_count, fp);
	}
	u32 oldpos = ftell(fp);
	fseek(fp, entryloc, SEEK_SET);
	WriteBRLANTagEntries(entry, head->entry_count, fp);
	fseek(fp, oldpos, SEEK_SET);
	u32 filesz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	entr->anim_header_len = sizeof(tag_header) + (sizeof(tag_entry) * head->entry_count);
	WriteBRLANEntry(entr, fp);
	*blobsize = filesz;
	*tagblob = (u8*)malloc(*blobsize);
	fseek(fp, 0, SEEK_SET);
	fread(*tagblob, *blobsize, 1, fp);
	free(entry);
	free(entryinfo);
	free(data);
	fclose(fp);
	remove("temp.blan");
	return filesz;
}
*/

/*
void create_tag_from_xml(mxml_node_t *tree, mxml_node_t *node, u8** tagblob, u32* blobsize)
{

//	tag_header head;
	brlyt_entry entry;
	char temp[256];
	memset(entry.name, 0, 20);
//        if(mxmlElementGetAttr(node, "name") != NULL)
//                strcpy(entr.name, mxmlElementGetAttr(node, "name"));
//        else{
//        }
	if(mxmlElementGetAttr(node, "type") != NULL)
		strcpy(temp, mxmlElementGetAttr(node, "type"));
	else{
		printf("No type attribute found!\nQuitting!\n");
		exit(1);
	}
	entry.magic[0] = temp[0];
	entry.magic[1] = temp[1];
	entry.magic[2] = temp[2];
	entry.magic[3] = temp[3];
	entry.length = 0;
	entry.data_location = 0;

//	if(mxmlElementGetAttr(node, "format") != NULL)
//		strcpy(temp, mxmlElementGetAttr(node, "format"));
//	else{
//		printf("No format attribute found!\nQuitting!\n");
//		exit(1);
//	}
	int x;
//	for(x = 0; x < strlen(temp); x++)
//		temp[x] = toupper(temp[x]);
//	if(strcmp(temp, "NORMAL") == 0)
//		entr.flags = 0x01000000;
//	else
//		entr.flags = atoi(temp);
	create_entries_from_xml(tree, node, &entr, &head, tagblob, blobsize);
}
*/

void WriteBRLYTEntry(mxml_node_t *tree, mxml_node_t *node, u8** tagblob, u32* blobsize, char temp[4])
{

	printf("temp holds: %c%c%c%c\t", temp[0], temp[1], temp[2], temp[3]);

	char lyt1[4] = {'l', 'y', 't', '1'};		//	//
	char txl1[4] = {'t', 'x', 'l', '1'};		//	//
	char fnl1[4] = {'f', 'n', 'l', '1'};		//	//
	char mat1[4] = {'m', 'a', 't', '1'};
	char pan1[4] = {'p', 'a', 'n', '1'};		//	//
	char wnd1[4] = {'w', 'n', 'd', '1'};		//	//
	char bnd1[4] = {'b', 'n', 'd', '1'};		//	//
	char pic1[4] = {'p', 'i', 'c', '1'};		//	/
	char txt1[4] = {'t', 'x', 't', '1'};		//	/
	char grp1[4] = {'g', 'r', 'p', '1'};		//	//
	char grs1[4] = {'g', 'r', 's', '1'};		
	char gre1[4] = {'g', 'r', 'e', '1'};		
	char pas1[4] = {'p', 'a', 's', '1'};		
	char pae1[4] = {'p', 'a', 'e', '1'};		
//	char lyt1[4] = {'l', 'y', 't', '1'};
//	char lyt1[4] = {'l', 'y', 't', '1'};
//	char lyt1[4] = {'l', 'y', 't', '1'};
//	char lyt1[4] = {'l', 'y', 't', '1'};

	if ( memcmp(temp, lyt1, sizeof(lyt1)) == 0)
	{
		brlyt_lytheader_chunk lytheader;
		printf("found a lyt1\n");
		mxml_node_t *subnode = mxmlFindElement(node , node , "a", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			printf("an a value found\n");
			char tempChar[4];
			get_value(subnode, tempChar, 256);
			//*(u32*)(&(data[x][i].part3)) = atoi(temp);
			printf("temp holds: %s\n", tempChar);
			lytheader.a = atoi(tempChar);
			lytheader.pad[0]=0;lytheader.pad[1]=0;lytheader.pad[2]=0;
			printf("a value: %08x\n", lytheader.a);
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode = mxmlFindElement(subnode , subnode , "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[4];
				get_value(valnode, tempChar, 256);
				float something;
				*(float*)(&(lytheader.width)) = atof(tempChar);
				printf("width: %f\n", lytheader.width);
			}
			valnode = mxmlFindElement(subnode , subnode ,"height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[4];
				get_value(valnode, tempChar, 256);
				//*(f32*)(&(data[x][i].part3)) = atof(tempChar);
				//float heightF = atof(tempChar);
				//printf("height: %f\n", heightF);
				*(float*)(&(lytheader.height)) = atof(tempChar);
				printf("height: %f\n", lytheader.height);
			}
		}
	}
	if ( memcmp(temp, txl1, sizeof(txl1)) == 0)
	{
		// create a numoffs for before txl names
		// brlut_numoffs_chumk chunk;
		// fwrite(chunk, f,f fp);
	
		//brlyt_group_chunk chunk;
		//chunk.unk = 0;
		printf("found a txl1\n");

//		if(mxmlElementGetAttr(node, "name") != NULL)
//			strcpy(temp, mxmlElementGetAttr(node, "name"));
//		else{
//			printf("No name attribute found!\nQuitting!\n");
//			exit(1);
//		}
//		strcpy(chunk.name, temp);

		mxml_node_t *subnode = mxmlFindElement(node, node, "entries", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			int numEntries = 0;
			// do a for loop to get chunk.numsubs

			mxml_node_t *valnode;
			for(valnode = mxmlFindElement(subnode, subnode, "name", NULL, NULL, MXML_DESCEND); valnode != NULL; valnode = mxmlFindElement(valnode, subnode, "name", NULL, NULL, MXML_DESCEND)) {

				if (valnode != NULL)
				{
					char tempSub[256];
					get_value(valnode, tempSub, 256);
				
					printf("name: %s\t", tempSub);
					numEntries++;
				}
			}
		}
	}
	if ( memcmp(temp, fnl1, sizeof(fnl1)) == 0)
	{
		// create a numoffs for before fnl names
		// brlut_numoffs_chumk chunk;
		// fwrite(chunk, f,f fp);
	
		//brlyt_group_chunk chunk;
		//chunk.unk = 0;
		printf("found a fnl1\n");

		mxml_node_t *subnode = mxmlFindElement(node, node, "entries", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			int numEntries = 0;
			// do a for loop to get chunk.numsubs

			mxml_node_t *valnode;
			for(valnode = mxmlFindElement(subnode, subnode, "name", NULL, NULL, MXML_DESCEND); valnode != NULL; valnode = mxmlFindElement(valnode, subnode, "name", NULL, NULL, MXML_DESCEND)) {

				if (valnode != NULL)
				{
					char tempSub[256];
					get_value(valnode, tempSub, 256);
				
					printf("name: %s\t", tempSub);
					numEntries++;
				}
			}
		}
	}
	if ( memcmp(temp, mat1, sizeof(mat1)) == 0)
	{

	}
	if ( memcmp(temp, pan1, sizeof(pan1)) == 0)
	{
		// create a pane chunk
		 brlyt_pane_chunk chunk;

		printf("found a pan1\n");

		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		strcpy(chunk.name, temp);
		printf("Name: %s\t", chunk.name);

		mxml_node_t *subnode = mxmlFindElement(node, node, "coords", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.x = atof(tempCoord);
				printf("x: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.y = atof(tempCoord);
				printf("y: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "z", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.z = atof(tempCoord);
				printf("z: %f\t", chunk.x);
			}
		}
		subnode = mxmlFindElement(node, node, "flip", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_x = atof(tempCoord);
				printf("flip_x: %f\t", chunk.flip_x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_y = atof(tempCoord);
				printf("flip_y: %f\t", chunk.flip_x);
			}
		}
		subnode = mxmlFindElement(node, node, "zoom", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.xmag = atof(tempCoord);
				printf("xmag: %f\t", chunk.xmag);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.ymag = atof(tempCoord);
				printf("ymag: %f\t", chunk.ymag);
			}
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.width = atof(tempCoord);
				printf("width: %f\t", chunk.width);
			}
			valnode = mxmlFindElement(subnode, subnode, "height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.height = atof(tempCoord);
				printf("height: %f\t", chunk.height);
			}
		}
		
		subnode = mxmlFindElement(node, node, "flags", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.flag1 = strtol(tempCoord, NULL, 16);
			chunk.flag2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("flag1: %08x\tflag2: %08x\t", chunk.flag1, chunk.flag2);
		}
		subnode = mxmlFindElement(node, node, "alpha", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.alpha = strtol(tempCoord, NULL, 16);
			chunk.alpha2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("alpha: %08x\talpha2: %08x\t", chunk.alpha, chunk.alpha2);
		}
		subnode = mxmlFindElement(node, node, "rotate", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.angle = atof(tempCoord);
			printf("angle: %08x\t", chunk.angle);
		}
	}
	if ( memcmp(temp, bnd1, sizeof(bnd1)) == 0)
	{
		// create a pane chunk
		 brlyt_pane_chunk chunk;
	
		printf("found a bnd1\n");

		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		strcpy(chunk.name, temp);
		printf("Name: %s\t", chunk.name);

		mxml_node_t *subnode = mxmlFindElement(node, node, "coords", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.x = atof(tempCoord);
				printf("x: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.y = atof(tempCoord);
				printf("y: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "z", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.z = atof(tempCoord);
				printf("z: %f\t", chunk.x);
			}
		}
		subnode = mxmlFindElement(node, node, "flip", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_x = atof(tempCoord);
				printf("flip_x: %f\t", chunk.flip_x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_y = atof(tempCoord);
				printf("flip_y: %f\t", chunk.flip_x);
			}
		}
		subnode = mxmlFindElement(node, node, "zoom", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.xmag = atof(tempCoord);
				printf("xmag: %f\t", chunk.xmag);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.ymag = atof(tempCoord);
				printf("ymag: %f\t", chunk.ymag);
			}
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.width = atof(tempCoord);
				printf("width: %f\t", chunk.width);
			}
			valnode = mxmlFindElement(subnode, subnode, "height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.height = atof(tempCoord);
				printf("height: %f\t", chunk.height);
			}
		}
		
		subnode = mxmlFindElement(node, node, "flags", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.flag1 = strtol(tempCoord, NULL, 16);
			chunk.flag2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("flag1: %08x\tflag2: %08x\t", chunk.flag1, chunk.flag2);
		}
		subnode = mxmlFindElement(node, node, "alpha", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.alpha = strtol(tempCoord, NULL, 16);
			chunk.alpha2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("alpha: %08x\talpha2: %08x\t", chunk.alpha, chunk.alpha2);
		}
		subnode = mxmlFindElement(node, node, "rotate", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.angle = atof(tempCoord);
			printf("angle: %08x\t", chunk.angle);
		}
	}
	if ( memcmp(temp, wnd1, sizeof(wnd1)) == 0)
	{
		// create a pane chunk
		 brlyt_pane_chunk chunk;
	
		printf("found a wnd1\n");

		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		strcpy(chunk.name, temp);
		printf("Name: %s\t", chunk.name);

		mxml_node_t *subnode = mxmlFindElement(node, node, "coords", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.x = atof(tempCoord);
				printf("x: %f\t", chunk.x);

			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.y = atof(tempCoord);
				printf("y: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "z", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.z = atof(tempCoord);
				printf("z: %f\t", chunk.x);
			}
		}
		subnode = mxmlFindElement(node, node, "flip", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_x = atof(tempCoord);
				printf("flip_x: %f\t", chunk.flip_x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_y = atof(tempCoord);
				printf("flip_y: %f\t", chunk.flip_x);
			}
		}
		subnode = mxmlFindElement(node, node, "zoom", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.xmag = atof(tempCoord);
				printf("xmag: %f\t", chunk.xmag);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.ymag = atof(tempCoord);
				printf("ymag: %f\t", chunk.ymag);
			}
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.width = atof(tempCoord);
				printf("width: %f\t", chunk.width);
			}
			valnode = mxmlFindElement(subnode, subnode, "height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.height = atof(tempCoord);
				printf("height: %f\t", chunk.height);
			}
		}
		
		subnode = mxmlFindElement(node, node, "flags", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.flag1 = strtol(tempCoord, NULL, 16);
			chunk.flag2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("flag1: %08x\tflag2: %08x\t", chunk.flag1, chunk.flag2);
		}
		subnode = mxmlFindElement(node, node, "alpha", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.alpha = strtol(tempCoord, NULL, 16);
			chunk.alpha2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("alpha: %08x\talpha2: %08x\t", chunk.alpha, chunk.alpha2);
		}
		subnode = mxmlFindElement(node, node, "rotate", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.angle = atof(tempCoord);
			printf("angle: %08x\t", chunk.angle);
		}
	}
	if ( memcmp(temp, txt1, sizeof(txt1)) == 0)
	{
		// create a pane chunk
		 brlyt_pane_chunk chunk;

		printf("found a txt1\n");

		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		strcpy(chunk.name, temp);
		printf("Name: %s\t", chunk.name);

		mxml_node_t *subnode = mxmlFindElement(node, node, "coords", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.x = atof(tempCoord);
				printf("x: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.y = atof(tempCoord);
				printf("y: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "z", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.z = atof(tempCoord);
				printf("z: %f\t", chunk.x);
			}
		}
		subnode = mxmlFindElement(node, node, "flip", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_x = atof(tempCoord);
				printf("flip_x: %f\t", chunk.flip_x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_y = atof(tempCoord);
				printf("flip_y: %f\t", chunk.flip_x);
			}
		}
		subnode = mxmlFindElement(node, node, "zoom", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.xmag = atof(tempCoord);
				printf("xmag: %f\t", chunk.xmag);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.ymag = atof(tempCoord);
				printf("ymag: %f\t", chunk.ymag);
			}
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.width = atof(tempCoord);
				printf("width: %f\t", chunk.width);
			}
			valnode = mxmlFindElement(subnode, subnode, "height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.height = atof(tempCoord);
				printf("height: %f\t", chunk.height);
			}
		}
		
		subnode = mxmlFindElement(node, node, "flags", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.flag1 = strtol(tempCoord, NULL, 16);
			chunk.flag2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("flag1: %08x\tflag2: %08x\t", chunk.flag1, chunk.flag2);
		}
		subnode = mxmlFindElement(node, node, "alpha", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.alpha = strtol(tempCoord, NULL, 16);
			chunk.alpha2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("alpha: %08x\talpha2: %08x\t", chunk.alpha, chunk.alpha2);
		}
		subnode = mxmlFindElement(node, node, "rotate", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.angle = atof(tempCoord);
			printf("angle: %08x\t", chunk.angle);
		}
//        u16 len1;			//
  //      u16 len2;			//
//        u16 mat_off;
 //       u16 font_idx;			/
 //       u8 unk4;
  //      u8 pad[3];      // [0, 0, 0]
   //     u32 name_offs;
    //    u32 color1;			//
     //   u32 color2;			//
      //  float font_size_x;		//
       // float font_size_y;		//
        //float char_space;		//
        //float line_space;		//
		brlyt_text_chunk chunk2;
		subnode = mxmlFindElement(node, node, "font", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			// get index
			char temp[256];
			if(mxmlElementGetAttr(subnode, "index") != NULL)
				strcpy(temp, mxmlElementGetAttr(subnode, "index"));
			else{
				printf("No index attribute found!\nQuitting!\n");
				exit(1);
			}
			//strcpy(chunk2.font_idx, temp);
			//chunk2.font_idx = strtol(temp, NULL, 10);
			chunk2.font_idx = atoi(temp);
			printf("Font_idx: %s\t", chunk2.font_idx);
			printf("font index string: %s", temp);

			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "xsize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.font_size_x = atof(tempCoord);
				printf("font_size_x: %f\t", chunk2.font_size_x);
			}
			valnode = mxmlFindElement(subnode, subnode, "ysize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.font_size_y = atof(tempCoord);
				printf("font_size_y: %f\t", chunk2.font_size_y);
			}
			valnode = mxmlFindElement(subnode, subnode, "charsize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.char_space = atof(tempCoord);
				printf("char_space: %f\t", chunk2.char_space);
			}
			valnode = mxmlFindElement(subnode, subnode, "linesize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.line_space = atof(tempCoord);
				printf("line_space: %f\t", chunk2.line_space);
			}
		}

		subnode = mxmlFindElement(node, node, "length", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk2.len1 = strtoul(tempCoord, NULL, 16);
			chunk2.len2 = strtoul(tempCoord+5, NULL, 16);
			printf("len1: %04x\tlen2: %04x\t", chunk2.len1, chunk2.len2);
		}
		subnode = mxmlFindElement(node, node, "color", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk2.color1 = strtoul(tempCoord, NULL, 16);
			chunk2.color2 = strtoul(tempCoord+9, NULL, 16);
			printf("color1: %08x\tcolor2: %08x\t", chunk2.color1, chunk2.color2);
		}

	}
	if ( memcmp(temp, pic1, sizeof(pic1)) == 0)
	{
		// create a pane chunk
		 brlyt_pane_chunk chunk;
	
		printf("found a pic1\n");

		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		strcpy(chunk.name, temp);
		printf("Name: %s\t", chunk.name);

		mxml_node_t *subnode = mxmlFindElement(node, node, "coords", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.x = atof(tempCoord);
				printf("x: %f\t", chunk.x);

			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.y = atof(tempCoord);
				printf("y: %f\t", chunk.x);
			}
			valnode = mxmlFindElement(subnode, subnode, "z", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.z = atof(tempCoord);
				printf("z: %f\t", chunk.x);
			}
		}
		subnode = mxmlFindElement(node, node, "flip", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_x = atof(tempCoord);
				printf("flip_x: %f\t", chunk.flip_x);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.flip_y = atof(tempCoord);
				printf("flip_y: %f\t", chunk.flip_x);
			}
		}
		subnode = mxmlFindElement(node, node, "zoom", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.xmag = atof(tempCoord);
				printf("xmag: %f\t", chunk.xmag);
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.ymag = atof(tempCoord);
				printf("ymag: %f\t", chunk.ymag);
			}
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.width = atof(tempCoord);
				printf("width: %f\t", chunk.width);
			}
			valnode = mxmlFindElement(subnode, subnode, "height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.height = atof(tempCoord);
				printf("height: %f\t", chunk.height);
			}
		}
		
		subnode = mxmlFindElement(node, node, "flags", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.flag1 = strtol(tempCoord, NULL, 16);
			chunk.flag2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("flag1: %08x\tflag2: %08x\t", chunk.flag1, chunk.flag2);
		}
		subnode = mxmlFindElement(node, node, "alpha", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.alpha = strtol(tempCoord, NULL, 16);
			chunk.alpha2 = strtol(&(tempCoord[9]), NULL, 16);
			printf("alpha: %08x\talpha2: %08x\t", chunk.alpha, chunk.alpha2);
		}
		subnode = mxmlFindElement(node, node, "rotate", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.angle = atof(tempCoord);
			printf("angle: %08x\t", chunk.angle);
		}
//        u32 vtx_colors[4];      // [4294967295L, 4294967295L, 4294967295L, 4294967295L]
//        u16 mat_off;
//        u8 num_texcoords;			//
//        u8 padding;             // 0
		brlyt_pic_chunk chunk2;
		subnode = mxmlFindElement(node, node, "colors", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			int i;
			for (i=0;i<4; i++)
			{
				valnode = mxmlFindElement(subnode, subnode, "vtx", NULL, NULL, MXML_DESCEND);
				if (valnode != NULL)
				{
					char tempCoord[256];
					get_value(valnode, tempCoord, 256);
					chunk2.vtx_colors[i] = strtoul(&(tempCoord[2]), NULL, 16);
					printf("vtx_colors: %08x\t", chunk2.vtx_colors[i]);
				}
			}
		}
		subnode = mxmlFindElement(node, node, "coordinates", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *setnode;
			setnode = mxmlFindElement(subnode, subnode, "set", NULL, NULL, MXML_DESCEND);
			if (setnode != NULL)
			{
				mxml_node_t *valnode;
				int numberOfCoordinates;
				for (valnode=mxmlFindElement(setnode, setnode, "coord", NULL, NULL, MXML_DESCEND) ; valnode != NULL  ; valnode=mxmlFindElement(valnode, setnode, "coord", NULL, NULL, MXML_DESCEND) )
				{
						char tempCoord[256];
						get_value(valnode, tempCoord, 256);
						//chunk2.vtx_colors[i] = strtoul(tempCoord, NULL, 16);
						float coordGotten = atof(tempCoord);
						printf("coord: %f\t", coordGotten);
				}
			}
		}
	}
	if ( memcmp(temp, grp1, sizeof(grp1)) == 0)
	{


		brlyt_group_chunk chunk;
		chunk.unk = 0;
		printf("found a grp1\n");

		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		strcpy(chunk.name, temp);

		mxml_node_t *subnode = mxmlFindElement(node, node, "subs", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			int numSubs = 0;
			// do a for loop to get chunk.numsubs

			mxml_node_t *valnode;
			for(valnode = mxmlFindElement(subnode, subnode, "sub", NULL, NULL, MXML_DESCEND); valnode != NULL; valnode = mxmlFindElement(valnode, subnode, "sub", NULL, NULL, MXML_DESCEND)) {
			if (valnode != NULL)
			{
					char tempSub[256];
					get_value(valnode, tempSub, 256);
				
					printf("sub: %s\t", tempSub);
					numSubs++;
				}
			}
		}
	}
	printf("\n");
}

void WriteBRLYTHeader(brlyt_header rlythead, FILE* fp)
{
	brlyt_header writehead;
	writehead.magic[0] = rlythead.magic[0];
	writehead.magic[1] = rlythead.magic[1];
	writehead.magic[2] = rlythead.magic[2];
	writehead.magic[3] = rlythead.magic[3];
	writehead.unk1 = be32(rlythead.unk1);
	writehead.filesize = be32(rlythead.filesize);
	writehead.lyt_offset = be16(rlythead.lyt_offset);
	writehead.unk2 = be16(rlythead.unk2);
	fwrite(&writehead, sizeof(brlyt_header), 1, fp);
}

void write_brlyt(char *infile, char *outfile)
{
	// set up tags list ??

	FILE* fpx = fopen(infile, "r");
	if(fpx == NULL) {
		printf("xmlyt couldn't be opened!\n");
		exit(1);
	}
	mxml_node_t *hightree = mxmlLoadFile(NULL, fpx, MXML_TEXT_CALLBACK);
	if(hightree == NULL) {
		printf("Couldn't open hightree!\n");
		exit(1);
	}
	mxml_node_t *tree = mxmlFindElement(hightree, hightree, "xmlyt", NULL, NULL, MXML_DESCEND);
	if(hightree == NULL) {
		printf("Couldn't get tree!\n");
		exit(1);
	}
	mxml_node_t *node;
	FILE* fp = fopen(outfile, "wb+");
	if(fpx == NULL) {
		printf("destination brlyt couldn't be opened!\n");
		exit(1);
	}
	u8* tagblob;
	u32 blobsize;
	u16 blobcount = 0;
	u32 bloboffset;
	brlyt_header rlythead;
        rlythead.magic[0] = 'R';
        rlythead.magic[1] = 'L';
        rlythead.magic[2] = 'Y';
        rlythead.magic[3] = 'T';
        rlythead.unk1 = 0xFEFF0008;
        rlythead.filesize = 0;
        rlythead.lyt_offset = sizeof(brlyt_header);
        rlythead.unk2 = 1;
        WriteBRLYTHeader(rlythead, fp);
	char temp[256];
//	big = (u8*)calloc(MAXIMUM_TAGS_SIZE, 1);
//	MEMORY* tagsmem = mopen(tagchunksbig, MAXIMUM_TAGS_SIZE, 3);
//	u32 totaltagsize = 0;

//	printf("\x1b[33mTemp holds: \x1b[31m%s", temp);

	for(node = mxmlFindElement(tree, tree, "tag", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, tree, "tag", NULL, NULL, MXML_DESCEND)) {

		char tempType[4];
		if(mxmlElementGetAttr(node, "type") != NULL)
			strcpy(tempType, mxmlElementGetAttr(node, "type"));
		else{
			printf("No type attribute found!\n");
			exit(1);
		}

		WriteBRLYTEntry(tree, node, &tagblob, &blobsize, tempType);

/*
		char lyt1[4] = {'l', 'y', 't', '1'};
		if ( memcmp(temp, lyt1, sizeof(lyt1)) == 0)
		{
			brlyt_lytheader_chunk lytheader;
			printf("found a lyt1\n");
			mxml_node_t *subnode = mxmlFindElement(node , node , "a", NULL, NULL, MXML_DESCEND);
			if (subnode != NULL)
			{
				printf("an a value found\n");
				char tempChar[4];
				get_value(subnode, tempChar, 256);
				//*(u32*)(&(data[x][i].part3)) = atoi(temp);
				printf("temp holds: %s\n", tempChar);
				lytheader.a = atoi(tempChar);
				lytheader.pad[0]=0;lytheader.pad[1]=0;lytheader.pad[2]=0;
				printf("a value: %08x\n", lytheader.a);
			}
			subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
			if (subnode != NULL)
			{
				//mxml_node_t *valnode = mxmlfindElemt(subnode, subnode, "height", NULL, NULL, MXML_DESCEND);
				mxml_node_t *valnode = mxmlFindElement(subnode , subnode , "width", NULL, NULL, MXML_DESCEND);
				if (valnode != NULL)
				{
					char tempChar[4];
					get_value(valnode, tempChar, 256);
					//*(f32*)(&(data[x][i].part3)) = atof(tempChar);
					//float widthF = atof(tempChar);
					float something;
					//printf("width: %f\n", widthF);
					*(float*)(&(lytheader.width)) = atof(tempChar);
					printf("width: %f\n", lytheader.width);
				}
                                valnode = mxmlFindElement(subnode , subnode ,"height", NULL, NULL, MXML_DESCEND);
                                if (valnode != NULL)
                                {
                                        char tempChar[4];
                                        get_value(valnode, tempChar, 256);
                                        //*(f32*)(&(data[x][i].part3)) = atof(tempChar);
                                        //float heightF = atof(tempChar);
					//printf("height: %f\n", heightF);
					*(float*)(&(lytheader.height)) = atof(tempChar);
                                        printf("height: %f\n", lytheader.height);
                                }
			}
		}
*/

//		printf("name: %s\n", temp);

//		brlyt_entry_header entry;
//		entry.magic[0] = temp[0];
//		entry.magic[1] = temp[1];
//		entry.magic[2] = temp[2];
//		entry.magic[3] = temp[3];
//		entry.length = 0;

		blobcount++;
//		bloboffset = ftell(fp) + mtell(tagsmem) - (4 * (blobcount + 1));
//		bloboffset = be32(bloboffset);
//		fwrite(&bloboffset, sizeof(u32), 1, fp);
//		create_tag_from_xml(tree, node, &tagblob, &blobsize);
//		mwrite(tagblob, blobsize, 1, tagsmem);
//		totaltagsize += blobsize;
	}
//	tagchunksbig = (u8*)mclose(tagsmem);
//	timgchunksbig = (u8*)mclose(timgmem);
//	fwrite(timgchunksbig, totaltimgize, 1, fp);
//	fwrite(tagchunksbig, totaltagsize, 1, fp);
//	fseek(fp, 0, SEEK_END);
//	lythead.size = ftell(fp) - rlythead.lyt_offset;
//	rlythead.file_size = ftell(fp);
//	fseek(fp, 0, SEEK_SET);
//	WriteBRLYTHeader(rlythead, fp);

	printf("blob count: %08x", blobcount);
	printf("\x1b[0m");
	fclose(fpx);
	fclose(fp);	
}

void make_brlyt(char* infile, char* outfile)
{
	printf("\x1b[33mParsing XMLYT @ \x1b[0m%s.\n", infile);
	write_brlyt(infile, outfile);
}



