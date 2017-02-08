#ifndef BSPLINE_H
#define BSPLINE_H

#include <vector>
#include <Eigen/Dense>

class BSpline
{
public:
	BSpline();

	void addControlPoint (const Eigen::Vector3f&);
	void setBasis        (const Eigen::Matrix4f&);

	Eigen::Vector3f position  (float t) const;
	Eigen::Vector3f tangent   (float t) const;
	Eigen::Vector3f curvature (float t) const;
private:
	Eigen::Matrix4f              m_basis;
	Eigen::Matrix4f              m_tangentBasis;
	Eigen::Matrix4f              m_curvatureBasis;
	std::vector<Eigen::Vector3f> m_controlPoints;

	void buildDerivativeBases();

	enum EvalType
	{
		POSITION,
		TANGENT,
		CURVATURE
	};
	Eigen::Vector3f evaluate(float t, EvalType) const;
};
#endif // BSPLINE_H
