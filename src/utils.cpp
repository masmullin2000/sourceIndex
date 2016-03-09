#include <cstdint>
#include "codepoints.h"

bool
IsIdentifierNonDigit
(
  uint32_t cp
)
{
  switch( cp )
  {
    case a_CHAR: case A_CHAR:
    case b_CHAR: case B_CHAR:
    case c_CHAR: case C_CHAR:
    case d_CHAR: case D_CHAR:
    case e_CHAR: case E_CHAR:
    case f_CHAR: case F_CHAR:
    case g_CHAR: case G_CHAR:
    case h_CHAR: case H_CHAR:
    case i_CHAR: case I_CHAR:
    case j_CHAR: case J_CHAR:
    case k_CHAR: case K_CHAR:
    case l_CHAR: case L_CHAR:
    case m_CHAR: case M_CHAR:
    case n_CHAR: case N_CHAR:
    case o_CHAR: case O_CHAR:
    case p_CHAR: case P_CHAR:
    case q_CHAR: case Q_CHAR:
    case r_CHAR: case R_CHAR:
    case s_CHAR: case S_CHAR:
    case t_CHAR: case T_CHAR:
    case u_CHAR: case U_CHAR:
    case v_CHAR: case V_CHAR:
    case w_CHAR: case W_CHAR:
    case x_CHAR: case X_CHAR:
    case y_CHAR: case Y_CHAR:
    case z_CHAR: case Z_CHAR:
    case UNDERSCORE:
      return true;
  }

  return false;
}

bool
IsDigit
(
  uint32_t cp
)
{
  switch( cp )
  {
    case ZERO_CHAR:
    case ONE_CHAR:
    case TWO_CHAR:
    case THREE_CHAR:
    case FOUR_CHAR:
    case FIVE_CHAR:
    case SIX_CHAR:
    case SEVEN_CHAR:
    case EIGHT_CHAR:
    case NINE_CHAR:
      return true;
  }

  return false;
}