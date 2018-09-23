#include "hud.h"
#include "cl_util.h"

#ifndef FX_PI
#define FX_PI 3.141592
#endif

#define BOUND_VALUE(var, min, max) if ((var) > (max)) { (var) = (max); }; if ((var) < (min)) { (var) = (min); }

inline float VectorAngle(const vec3_t a, const vec3_t b)
{
	float length_a = a.Length();
	float length_b = b.Length();
	float length_ab = length_a * length_b;

	if (length_ab == 0.0)
		return 0.0;

	return (double)(acos(DotProduct(a, b) / length_ab) * (180 / FX_PI));
}

void MakeVector(const vec3_t ain, vec3_t &vout)
{
	float pitch;
	float yaw;
	float tmp;

	pitch = (ain[0] * FX_PI / 180);
	yaw = (ain[1] * FX_PI / 180);
	tmp = cos(pitch);

	vout[0] = (-tmp * -cos(yaw));
	vout[1] = (sin(yaw) * tmp);
	vout[2] = -sin(pitch);
}

void VectorRotateX(const vec3_t in, float angle, vec3_t &out)
{
	float a, c, s;

	a = (angle * FX_PI / 180);
	c = cos(a);
	s = sin(a);

	out[0] = in[0];
	out[1] = c * in[1] - s * in[2];
	out[2] = s * in[1] + c * in[2];
}

void VectorRotateY(const vec3_t in, float angle, vec3_t &out)
{
	float a, c, s;

	a = (angle * FX_PI / 180);
	c = cos(a);
	s = sin(a);

	out[0] = c * in[0] + s * in[2];
	out[1] = in[1];
	out[2] = -s * in[0] + c * in[2];
}

void VectorRotateZ(const vec3_t in, float angle, vec3_t &out)
{
	float a, c, s;

	a = (angle * FX_PI / 180);
	c = cos(a);
	s = sin(a);

	out[0] = c * in[0] - s * in[1];
	out[1] = s * in[0] + c * in[1];
	out[2] = in[2];
}

extern vec3_t v_origin;		// last view origin
extern vec3_t v_angles;		// last view angle
extern vec3_t v_cl_angles;	// last client/mouse angle
extern vec3_t v_sim_org;	// last sim origin

int CalcScreen(const float in[3], float out[2])
{
	vec3_t aim;
	vec3_t newaim;
	vec3_t view;
	vec3_t tmp;
	float num;

	aim = Vector(in) - Vector(v_origin);
	MakeVector(v_angles, view);

	if (VectorAngle(view, aim) > (gHUD.m_iFOV / 1.8))
		return false;

	VectorRotateZ(aim, -v_angles[1], newaim);
	VectorRotateY(newaim, -v_angles[0], tmp);
	VectorRotateX(tmp, -v_angles[2], newaim);
	newaim[1] *= 0.733333f;
	newaim[2] *= 0.733333f;

	if (gHUD.m_iFOV == 0.0)
		return false;

	num = (((ScreenWidth / 2) / newaim[0]) * (120.0 / gHUD.m_iFOV - 1.0 / 3.0));
	out[0] = (ScreenWidth / 2) - num * newaim[1];
	out[1] = (ScreenHeight / 2) - num * newaim[2];

	BOUND_VALUE(out[0], 0, (ScreenWidth / 2) * 2);
	BOUND_VALUE(out[1], 0, (ScreenHeight / 2) * 2);
	return true;
}
