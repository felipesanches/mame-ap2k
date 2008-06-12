/***************************************************************************

Blades of Steel(GX797) (c) 1987 Konami

Driver by Manuel Abadia <manu@teleline.es>

Interrupts:

    CPU #0 (6309):
    --------------
    * IRQ: not used.
    * FIRQ: generated by VBLANK.
    * NMI: writes the sound command to the 6809.

    CPU #1 (6809):
    --------------
    * IRQ: triggered by the 6309 when a sound command is written.
    * FIRQ: not used.
    * NMI: not used.

Notes:
    * The protection is not fully understood(Konami 051733). The
    game is playable, but is not 100% accurate.
    * Missing samples.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m6809/m6809.h"
#include "cpu/hd6309/hd6309.h"
#include "video/konamiic.h"
#include "sound/2203intf.h"
#include "sound/upd7759.h"

/* from video */
int bladestl_spritebank;
PALETTE_INIT( bladestl );
VIDEO_START( bladestl );
VIDEO_UPDATE( bladestl );
WRITE8_HANDLER( bladestl_vreg_w );

static INTERRUPT_GEN( bladestl_interrupt )
{
	if (cpu_getiloops() == 0){
		if (K007342_is_INT_enabled())
			cpunum_set_input_line(machine, 0, HD6309_FIRQ_LINE, HOLD_LINE);
	}
	else if (cpu_getiloops() % 2){
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static READ8_HANDLER( trackball_r )
{
	static int last[4];
	int curr,delta;
	static const char *port[] = { "TRACKBALL_P1_1", "TRACKBALL_P1_2", "TRACKBALL_P2_1", "TRACKBALL_P1_2" };

	curr = input_port_read(machine, port[offset]);
	delta = (curr - last[offset]) & 0xff;
	last[offset] = curr;
	return (delta & 0x80) | (curr >> 1);
}

static WRITE8_HANDLER( bladestl_bankswitch_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int bankaddress;

	/* bits 0 & 1 = coin counters */
	coin_counter_w(0,data & 0x01);
	coin_counter_w(1,data & 0x02);

	/* bits 2 & 3 = lamps */
	set_led_status(0,data & 0x04);
	set_led_status(1,data & 0x08);

	/* bit 4 = relay (???) */

	/* bits 5-6 = bank number */
	bankaddress = 0x10000 + ((data & 0x60) >> 5) * 0x2000;
	memory_set_bankptr(1,&RAM[bankaddress]);

	/* bit 7 = select sprite bank */
	bladestl_spritebank = (data & 0x80) << 3;

}

static WRITE8_HANDLER( bladestl_sh_irqtrigger_w )
{
	soundlatch_w(machine, offset, data);
	cpunum_set_input_line(machine, 1, M6809_IRQ_LINE, HOLD_LINE);
	//logerror("(sound) write %02x\n", data);
}

static WRITE8_HANDLER( bladestl_port_B_w ){
	/* bit 1, 2 unknown */
	upd7759_set_bank_base(0, ((data & 0x38) >> 3)*0x20000);
}

static WRITE8_HANDLER( bladestl_speech_ctrl_w ){
	upd7759_reset_w(0, data & 1);
	upd7759_start_w(0, data & 2);
}

static ADDRESS_MAP_START( bladestl_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(K007342_r)			/* Color RAM + Video RAM */
	AM_RANGE(0x2000, 0x21ff) AM_READ(K007420_r)			/* Sprite RAM */
	AM_RANGE(0x2200, 0x23ff) AM_READ(K007342_scroll_r)	/* Scroll RAM */
	AM_RANGE(0x2400, 0x245f) AM_READ(SMH_RAM)			/* Palette */
	AM_RANGE(0x2e01, 0x2e01) AM_READ_PORT("P1")			/* 1P controls */
	AM_RANGE(0x2e02, 0x2e02) AM_READ_PORT("P2")			/* 2P controls */
	AM_RANGE(0x2e03, 0x2e03) AM_READ_PORT("DSW2")		/* DISPW #2 */
	AM_RANGE(0x2e40, 0x2e40) AM_READ_PORT("DSW1")		/* DIPSW #1 */
	AM_RANGE(0x2e00, 0x2e00) AM_READ_PORT("COINSW")		/* DIPSW #3, coinsw, startsw */
	AM_RANGE(0x2f00, 0x2f03) AM_READ(trackball_r)		/* Trackballs */
	AM_RANGE(0x2f80, 0x2f9f) AM_READ(K051733_r)			/* Protection: 051733 */
	AM_RANGE(0x4000, 0x5fff) AM_READ(SMH_RAM)			/* Work RAM */
	AM_RANGE(0x6000, 0x7fff) AM_READ(SMH_BANK1)			/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_ROM)			/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bladestl_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(K007342_w)				/* Color RAM + Video RAM */
	AM_RANGE(0x2000, 0x21ff) AM_WRITE(K007420_w)				/* Sprite RAM */
	AM_RANGE(0x2200, 0x23ff) AM_WRITE(K007342_scroll_w)		/* Scroll RAM */
	AM_RANGE(0x2400, 0x245f) AM_WRITE(SMH_RAM) AM_BASE(&paletteram)/* palette */
	AM_RANGE(0x2600, 0x2607) AM_WRITE(K007342_vreg_w)			/* Video Registers */
	AM_RANGE(0x2e80, 0x2e80) AM_WRITE(bladestl_sh_irqtrigger_w)/* cause interrupt on audio CPU */
	AM_RANGE(0x2ec0, 0x2ec0) AM_WRITE(watchdog_reset_w)		/* watchdog reset */
	AM_RANGE(0x2f40, 0x2f40) AM_WRITE(bladestl_bankswitch_w)	/* bankswitch control */
	AM_RANGE(0x2f80, 0x2f9f) AM_WRITE(K051733_w)				/* Protection: 051733 */
	AM_RANGE(0x2fc0, 0x2fc0) AM_WRITE(SMH_NOP)				/* ??? */
	AM_RANGE(0x4000, 0x5fff) AM_WRITE(SMH_RAM)				/* Work RAM */
	AM_RANGE(0x6000, 0x7fff) AM_WRITE(SMH_RAM)				/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_WRITE(SMH_ROM)				/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bladestl_readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READ(SMH_RAM)				/* RAM */
	AM_RANGE(0x1000, 0x1000) AM_READ(YM2203_status_port_0_r)	/* YM2203 */
	AM_RANGE(0x1001, 0x1001) AM_READ(YM2203_read_port_0_r)	/* YM2203 */
	AM_RANGE(0x4000, 0x4000) AM_READ(upd7759_0_busy_r)		/* UPD7759 */
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)			/* soundlatch_r */
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_ROM)				/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bladestl_writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_WRITE(SMH_RAM)				/* RAM */
	AM_RANGE(0x1000, 0x1000) AM_WRITE(YM2203_control_port_0_w)/* YM2203 */
	AM_RANGE(0x1001, 0x1001) AM_WRITE(YM2203_write_port_0_w)	/* YM2203 */
	AM_RANGE(0x3000, 0x3000) AM_WRITE(bladestl_speech_ctrl_w)	/* UPD7759 */
	AM_RANGE(0x5000, 0x5000) AM_WRITE(SMH_NOP)				/* ??? */
	AM_RANGE(0x8000, 0xffff) AM_WRITE(SMH_ROM)				/* ROM */
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( bladestl )
	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

	PORT_START_TAG("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, "Bonus time set" )		PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30 secs" )
	PORT_DIPSETTING(    0x10, "20 secs" )
	PORT_DIPSETTING(    0x08, "15 secs" )
	PORT_DIPSETTING(    0x00, "10 secs" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, "Difficult" )
	PORT_DIPSETTING(	0x00, "Very difficult" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("COINSW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:2" )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START_TAG("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_DIPNAME( 0x80, 0x80, "Period time set" )		PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x00, "7" )

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("TRACKBALL_P1_1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(1)

	PORT_START_TAG("TRACKBALL_P1_2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START_TAG("TRACKBALL_P2_1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(2)

	PORT_START_TAG("TRACKBALL_P2_2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( bladstle )
	PORT_INCLUDE( bladestl )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )	/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )	/* Listed as "Unused" */

	PORT_MODIFY("P1")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:4" )	/* Listed as "Unused" */

	PORT_MODIFY("TRACKBALL_P2_1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_MODIFY("TRACKBALL_P2_2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,			/* 8 x 8 characters */
	0x40000/32,		/* 8192 characters */
	4,				/* 4bpp */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8			/* every character takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,8,			/* 8*8 sprites */
	0x40000/32,		/* 8192 sprites */
	4,				/* 4 bpp */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8			/* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( bladestl )
	GFXDECODE_ENTRY( REGION_GFX1, 0x000000, charlayout,     0,	2 )	/* colors 00..31 */
	GFXDECODE_ENTRY( REGION_GFX1, 0x040000, spritelayout,   32,	16 )	/* colors 32..47 but using lookup table */
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

static const struct YM2203interface ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		NULL,
		NULL,
		upd7759_0_port_w,
		bladestl_port_B_w
	},
	NULL
};

static const struct upd7759_interface upd7759_interface =
{
	REGION_SOUND1					/* memory regions */
};

static MACHINE_DRIVER_START( bladestl )

	/* basic machine hardware */
	MDRV_CPU_ADD(HD6309, 24000000/2)		/* 24MHz/2 (?) */
	MDRV_CPU_PROGRAM_MAP(bladestl_readmem,bladestl_writemem)
	MDRV_CPU_VBLANK_INT_HACK(bladestl_interrupt,2) /* (1 IRQ + 1 NMI) */

	MDRV_CPU_ADD(M6809, 2000000)
	/* audio CPU */		/* ? */
	MDRV_CPU_PROGRAM_MAP(bladestl_readmem_sound,bladestl_writemem_sound)

	MDRV_INTERLEAVE(10)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(bladestl)
	MDRV_PALETTE_INIT(bladestl)
	MDRV_PALETTE_LENGTH(32 + 16*16)

	MDRV_VIDEO_START(bladestl)
	MDRV_VIDEO_UPDATE(bladestl)

	/* sound hardware */
	/* the initialization order is important, the port callbacks being
       called at initialization time */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_CONFIG(upd7759_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MDRV_SOUND_ADD(YM2203, 3579545)
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( bladestl )
	ROM_REGION( 0x18000, REGION_CPU1, 0 ) /* code + banked roms */
	ROM_LOAD( "797t01.bin", 0x10000, 0x08000, CRC(89d7185d) SHA1(0d2f346d9515cab0389106c0e227fb0bd84a2c9c) )	/* fixed ROM */
	ROM_CONTINUE(			0x08000, 0x08000 )				/* banked ROM */

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "797c02", 0x08000, 0x08000, CRC(65a331ea) SHA1(f206f6c5f0474542a5b7686b2f4d2cc7077dd5b9) )

	ROM_REGION( 0x080000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "797a05",	0x000000, 0x40000, CRC(5491ba28) SHA1(c807774827c55c211ab68f548e1e835289cc5744) )	/* tiles */
	ROM_LOAD( "797a06",	0x040000, 0x40000, CRC(d055f5cc) SHA1(3723b39b2a3e6dd8e7fc66bbfe1eef9f80818774) )	/* sprites */

	ROM_REGION( 0x0100, REGION_PROMS, 0 )
	ROM_LOAD( "797a07", 0x0000, 0x0100, CRC(7aecad4e) SHA1(05150a8dd25bdd6ab0c5b350e6ffd272f040e46a) ) /* sprites lookup table */

	ROM_REGION( 0xc0000, REGION_SOUND1, 0 ) /* uPD7759 data (chip 1) */
	ROM_LOAD( "797a03", 0x00000, 0x80000, CRC(9ee1a542) SHA1(c9a142a326875a50f03e49e83a84af8bb423a467) )
	ROM_LOAD( "797a04",	0x80000, 0x40000, CRC(9ac8ea4e) SHA1(9f81eff970c9e8aea6f67d8a7d89805fae044ae1) )
ROM_END

ROM_START( bladstle )
	ROM_REGION( 0x18000, REGION_CPU1, 0 ) /* code + banked roms */
	ROM_LOAD( "797e01", 0x10000, 0x08000, CRC(f8472e95) SHA1(8b6caa905fb1642300dd9da508871b00429872c3) )	/* fixed ROM */
	ROM_CONTINUE(		0x08000, 0x08000 )				/* banked ROM */

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "797c02", 0x08000, 0x08000, CRC(65a331ea) SHA1(f206f6c5f0474542a5b7686b2f4d2cc7077dd5b9) )

	ROM_REGION( 0x080000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "797a05",	0x000000, 0x40000, CRC(5491ba28) SHA1(c807774827c55c211ab68f548e1e835289cc5744) )	/* tiles */
	ROM_LOAD( "797a06",	0x040000, 0x40000, CRC(d055f5cc) SHA1(3723b39b2a3e6dd8e7fc66bbfe1eef9f80818774) )	/* sprites */

	ROM_REGION( 0x0100, REGION_PROMS, 0 )
	ROM_LOAD( "797a07", 0x0000, 0x0100, CRC(7aecad4e) SHA1(05150a8dd25bdd6ab0c5b350e6ffd272f040e46a) ) /* sprites lookup table */

	ROM_REGION( 0xc0000, REGION_SOUND1, 0 ) /* uPD7759 data */
	ROM_LOAD( "797a03", 0x00000, 0x80000, CRC(9ee1a542) SHA1(c9a142a326875a50f03e49e83a84af8bb423a467) )
	ROM_LOAD( "797a04",	0x80000, 0x40000, CRC(9ac8ea4e) SHA1(9f81eff970c9e8aea6f67d8a7d89805fae044ae1) )
ROM_END



GAME( 1987, bladestl, 0,        bladestl, bladestl, 0, ROT90, "Konami", "Blades of Steel (version T)", 0 )
GAME( 1987, bladstle, bladestl, bladestl, bladstle, 0, ROT90, "Konami", "Blades of Steel (version E)", 0 )
