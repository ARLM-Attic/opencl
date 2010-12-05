#include <AntTweakBar.h>
#include <cmath>

void TW_CALL CopyCDStringToClient(char **destPtr, const char *src);

void SetQuaternionFromAxisAngle(const float *axis, float angle, float *quat);
void ConvertQuaternionToMatrix(const float *quat, float *mat);
void MultiplyQuaternions(const float *q1, const float *q2, float *qout);