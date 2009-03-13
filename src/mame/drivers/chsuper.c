#include "driver.h"
#include "cpu/z180/z180.h"


static VIDEO_START(chsuper)
{
}

static VIDEO_UPDATE(chsuper)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	UINT8 *vram = memory_region(screen->machine, "vram");
	int count = 0x0000;
	int y,x;

	for (y=0;y<64;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = ((vram[count+1]<<8) | vram[count]) & 0x3fff;
			//int colour = tile>>12;

			drawgfx(bitmap,gfx,tile,0,0,0,x*8,y*8,cliprect,TRANSPARENCY_NONE,0);
			count+=2;
		}
	}

	return 0;
}

static WRITE8_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs;

	switch(offset)
	{
		case 0:
			pal_offs = data;
			break;
		case 2:
			internal_pal_offs = 0;
			break;
		case 1:
			switch(internal_pal_offs)
			{
				case 0:
					r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 1:
					g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 2:
					b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}

static READ8_HANDLER( ff_r )
{
	return 0xff;
}

static WRITE8_HANDLER( chsuper_vram_w )
{
	UINT8 *vram = memory_region(space->machine, "vram");

	vram[offset] = data;
}

static ADDRESS_MAP_START( chsuper_prg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0efff) AM_ROM
	AM_RANGE(0x00000, 0x01fff) AM_WRITE( chsuper_vram_w )
	AM_RANGE(0x0f000, 0x0ffff) AM_RAM AM_REGION("maincpu", 0xf000)
	AM_RANGE(0xfb000, 0xfbfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( chsuper_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs
	AM_RANGE( 0x00e8, 0x00e8 ) AM_READ( ff_r )
	AM_RANGE( 0x00fc, 0x00fe ) AM_WRITE( paletteram_io_w )
	AM_RANGE( 0xff00, 0xffff ) AM_RAM // unk writes
ADDRESS_MAP_END



static INPUT_PORTS_START( chsuper )

	PORT_START("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/* WRONG! */

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7},
	{ 2*8,3*8,0*8,1*8, 256+2*8, 256+3*8, 256+0*8, 256+1*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32},
	8*64
};

static GFXDECODE_START( chsuper )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   0, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( chsuper )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, XTAL_12MHz / 2)	/* HD64180RP8, 8 MHz? */
	MDRV_CPU_PROGRAM_MAP(chsuper_prg_map,0)
	MDRV_CPU_IO_MAP(chsuper_portmap,0)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0, 64*8-1)

	MDRV_GFXDECODE(chsuper)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(chsuper)
	MDRV_VIDEO_UPDATE(chsuper)

	/* sound hardware */
//	MDRV_SPEAKER_STANDARD_MONO("mono")

MACHINE_DRIVER_END


/*  ROM Regions definition
 */

ROM_START( chsuper3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "c.bin",  0x0000, 0x80000, CRC(e987ed1f) SHA1(8d1ee01914356714c7d1f8437d98b41a707a174a) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "a.bin",  0x00000, 0x80000, CRC(ace8b591) SHA1(e9ba5efebdc9b655056ed8b2621f062f50e0528f) )
	ROM_LOAD( "b.bin",  0x80000, 0x80000, CRC(5f58c722) SHA1(d339ae27af010b058eae9084fba85fb2fbed3952) )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )
ROM_END

ROM_START( chsuper2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "c.bin",  0x0000, 0x80000, CRC(cbf59e69) SHA1(68e4b167fdf9103fd748cff401f4fe7c1d214552) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "a.bin",  0x00000, 0x80000, CRC(7caa8ebe) SHA1(440306a208ec8afd570b15f05b5dc542acc98510) )
	ROM_LOAD( "b.bin",  0x80000, 0x80000, CRC(7bb463d7) SHA1(fb3842ba53e545fa47574c91df7281a9cb417395) )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )
ROM_END



GAME( 1999, chsuper3, 0,        chsuper, chsuper,  0, ROT0, "unknown",    "Champion Super 3", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1999, chsuper2, chsuper3, chsuper, chsuper,  0, ROT0, "unknown",    "Champion Super 2", GAME_NOT_WORKING|GAME_NO_SOUND )
