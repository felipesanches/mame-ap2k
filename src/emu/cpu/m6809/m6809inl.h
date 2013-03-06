/*********************************************************************

    m6809inl.h

	Portable 6809 emulator - Inline functions for the purposes of
	optimization

**********************************************************************/

#include "m6809.h"

//-------------------------------------------------
//  rotate_right
//-------------------------------------------------

template<class T>
inline ATTR_FORCE_INLINE T m6809_base_device::rotate_right(T value)
{
	bool new_carry = (value & 1) ? true : false;
	value = value >> 1;

	T high_bit = ((T) 1) << (sizeof(T) * 8 - 1);
	if (m_cc & CC_C)
		value |= high_bit;
	else
		value &= ~high_bit;

	if (new_carry)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
	return value;
}


//-------------------------------------------------
//  rotate_left
//-------------------------------------------------

template<class T>
inline ATTR_FORCE_INLINE UINT32 m6809_base_device::rotate_left(T value)
{
	T high_bit = ((T) 1) << (sizeof(T) * 8 - 1);
	bool new_carry = (value & high_bit) ? true : false;

	UINT32 new_value = value;
	new_value <<= 1;

	if (m_cc & CC_C)
		new_value |= 1;
	else
		new_value &= ~1;

	if (new_carry)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
	return new_value;
}


//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE UINT8 m6809_base_device::read_operand()
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:			return read_memory(m_ea.w);
		case ADDRESSING_MODE_IMMEDIATE:		return read_opcode_arg();
		case ADDRESSING_MODE_REGISTER_A:	return m_d.b.h;
		case ADDRESSING_MODE_REGISTER_B:	return m_d.b.l;
		default:							fatalerror("Unexpected");	return 0x00;
	}
}


//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE UINT8 m6809_base_device::read_operand(int ordinal)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:			return read_memory(m_ea.w + ordinal);
		case ADDRESSING_MODE_IMMEDIATE:		return read_opcode_arg();
		default:							fatalerror("Unexpected");	return 0x00;
	}
}


//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::write_operand(UINT8 data)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_IMMEDIATE:		/* do nothing */				break;
		case ADDRESSING_MODE_EA:			write_memory(m_ea.w, data);		break;
		case ADDRESSING_MODE_REGISTER_A:	m_d.b.h = data;					break;
		case ADDRESSING_MODE_REGISTER_B:	m_d.b.l = data;					break;
		default:							fatalerror("Unexpected");		break;
	}
}


//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::write_operand(int ordinal, UINT8 data)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_IMMEDIATE:		/* do nothing */				break;
		case ADDRESSING_MODE_EA:			write_memory(m_ea.w + ordinal, data);	break;
		default:							fatalerror("Unexpected");		break;
	}
}


//-------------------------------------------------
//  burn_any_delay_loops - optimization for delay
//	loops
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::burn_any_delay_loops()
{
	if ((m_opcode == 0x26)
		&& !(m_cc & CC_Z)
		&& !(machine().debug_flags & DEBUG_FLAG_CALL_HOOK)
		&& (read_opcode_arg(m_pc.w) == 0xFC)
		&& (read_opcode(m_pc.w - 3) == 0x30)
		&& (read_opcode_arg(m_pc.w - 2) == 0x1F))
	{
		// LEAX -1,X ; BNE *
		UINT16 burned_loops = MIN((int) m_x.w - 1, m_icount / 8);
		m_x.w -= burned_loops;
		eat(burned_loops * 8);
	}
}


//-------------------------------------------------
//  read_exgtfr_register
//-------------------------------------------------

inline ATTR_FORCE_INLINE m6809_base_device::exgtfr_register m6809_base_device::read_exgtfr_register(UINT8 reg)
{
	exgtfr_register result;
	result.byte_value = 0xFF;
	result.word_value = 0x00FF;

	switch(reg & 0x0F)
	{
		case  0: result.word_value = m_d.w;		break;	// D
		case  1: result.word_value = m_x.w;		break;	// X
		case  2: result.word_value = m_y.w;		break;	// Y
		case  3: result.word_value = m_u.w;		break;	// U
		case  4: result.word_value = m_s.w;		break;	// S
		case  5: result.word_value = m_pc.w;	break;	// PC
		case  8: result.byte_value = m_d.b.h;	break;	// A
		case  9: result.byte_value = m_d.b.l;	break;	// B
		case 10: result.byte_value = m_cc;		break;	// CC
		case 11: result.byte_value = m_dp;		break;	// DP
	}
	return result;
}


//-------------------------------------------------
//  write_exgtfr_register
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::write_exgtfr_register(UINT8 reg, m6809_base_device::exgtfr_register value)
{
	switch(reg & 0x0F)
	{
		case  0: m_d.w   = value.word_value;	break;	// D
		case  1: m_x.w   = value.word_value;	break;	// X
		case  2: m_y.w   = value.word_value;	break;	// Y
		case  3: m_u.w   = value.word_value;	break;	// U
		case  4: m_s.w   = value.word_value;	break;	// S
		case  5: m_pc.w  = value.word_value;	break;	// PC
		case  8: m_d.b.h = value.byte_value;	break;	// A
		case  9: m_d.b.l = value.byte_value;	break;	// B
		case 10: m_cc    = value.byte_value;	break;	// CC
		case 11: m_dp    = value.byte_value;	break;	// DP
	}
}


//-------------------------------------------------
//  daa - decimal arithmetic adjustment instruction
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::daa()
{
	UINT16 t, cf = 0;
	UINT8 msn = m_d.b.h & 0xF0;
	UINT8 lsn = m_d.b.h & 0x0F;

	// compute the carry
	if (lsn > 0x09 || m_cc & CC_H)	cf |= 0x06;
	if (msn > 0x80 && lsn > 0x09 )	cf |= 0x60;
	if (msn > 0x90 || m_cc & CC_C)	cf |= 0x60;

	// calculate the result
	t = m_d.b.h + cf;

	m_cc &= ~CC_V;
	if (t & 0x0100)		// keep carry from previous operation
		m_cc |= CC_C;

	// and put it back into A
	m_d.b.h = set_flags(CC_NZ, (UINT8) t);
}


//-------------------------------------------------
//  mul
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::mul()
{
	// perform multiply
	UINT16 result = ((UINT16) m_d.b.h) * ((UINT16) m_d.b.l);

	// set result and Z flag
	m_d.w = set_flags(CC_Z, result);

	// set C flag
	if (m_d.w & 0x0080)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
}


//-------------------------------------------------
//  ireg
//-------------------------------------------------

inline ATTR_FORCE_INLINE UINT16 &m6809_base_device::ireg()
{
	switch(m_opcode & 0x60)
	{
		case 0x00:	return m_x.w;
		case 0x20:	return m_y.w;
		case 0x40:	return m_u.w;
		case 0x60:	return m_s.w;
		default:
			fatalerror("Unexpected");
			return m_x.w;
	}
}


//-------------------------------------------------
//  set_flags
//-------------------------------------------------

template<class T>
inline T m6809_base_device::set_flags(UINT8 mask, T a, T b, UINT32 r)
{
	T hi_bit = (T) (1 << (sizeof(T) * 8 - 1));

	m_cc &= ~mask;
	if (mask & CC_H)
		m_cc |= ((a ^ b ^ r) & 0x10) ? CC_H : 0;
	if (mask & CC_N)
		m_cc |= (r & hi_bit) ? CC_N : 0;
	if (mask & CC_Z)
		m_cc |= (((T)r) == 0) ? CC_Z : 0;
	if (mask & CC_V)
		m_cc |= ((a ^ b ^ r ^ (r >> 1)) & hi_bit) ? CC_V : 0;
	if (mask & CC_C)
		m_cc |= (r & (hi_bit << 1)) ? CC_C : 0;
	return (T) r;
}


//-------------------------------------------------
//  set_flags
//-------------------------------------------------

template<class T>
inline T m6809_base_device::set_flags(UINT8 mask, T r)
{
	return set_flags(mask, (T)0, r, r);
}


//-------------------------------------------------
//  eat_remaining
//-------------------------------------------------

inline void m6809_base_device::eat_remaining()
{
	// we do this in order to be nice to people debugging
	UINT16 real_pc = m_pc.w;

	eat(m_icount);
	
	m_pc.w = m_ppc.w;
	debugger_instruction_hook(this, m_pc.w);
	m_pc.w = real_pc;
}



//-------------------------------------------------
//  is_register_addressing_mode
//-------------------------------------------------

inline bool m6809_base_device::is_register_addressing_mode()
{
	return (m_addressing_mode != ADDRESSING_MODE_IMMEDIATE)
		&& (m_addressing_mode != ADDRESSING_MODE_EA);
}



//-------------------------------------------------
//  get_pending_interrupt
//-------------------------------------------------

inline UINT16 m6809_base_device::get_pending_interrupt()
{
	UINT16 result;
	if (m_nmi_asserted)
		result = VECTOR_NMI;
	else if (!(m_cc & CC_F) && m_firq_line)
		result = VECTOR_FIRQ;
	else if (!(m_cc & CC_I) && m_irq_line)
		result = VECTOR_IRQ;
	else
		result = 0;
	return result;
}



//-------------------------------------------------
//  check_pending_interrupt
//-------------------------------------------------

inline UINT16 m6809_base_device::check_pending_interrupt()
{
	UINT16 result = get_pending_interrupt();

	// check_pending_interrupt() will also invoke the IRQ
	// callback for FIRQ and IRQ interrupts
	//
	// I'm not sure why this is necessary; neither the 6809, 6309
	// nor (presumably) Konami had any interrupt acknowledge lines so
	// it isn't clear what this is reflecting
	switch(result)
	{
		case VECTOR_FIRQ:
			standard_irq_callback(M6809_FIRQ_LINE);	
			break;
		case VECTOR_IRQ:
			standard_irq_callback(M6809_IRQ_LINE);	
			break;
	}

	return result;
}