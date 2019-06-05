#include "curve.h"
#include "extra.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <random>

using namespace std;

namespace
{
    // Approximately equal to.  We don't want to use == because of
    // precision issues with floating point.
    inline bool approx( const Vector3f& lhs, const Vector3f& rhs )
    {
        const float eps = 1e-8f;
        return ( lhs - rhs ).absSquared() < eps;
    }

    
}

// monomial basis (1, t, t^2, t^3)
Vector4f mono_basis(float t) {
	return Vector4f (1., t, pow(t, 2), pow(t, 3));
}

// derivative of monomial basis w.r.t parameter t
Vector4f mono_basis_prime(float t) {
	return Vector4f (0., 1., 2. * t, 3. * pow(t, 2));
}

// computing the coordinates for given a basis
Vector3f new_coordinates (Vector4f basis, Vector4f x, Vector4f y, Vector4f z) {
	return Vector3f (Vector4f::dot(basis, x), Vector4f::dot(basis, y), Vector4f::dot(basis, z));
}

    
// function that generates points of Bezier spline given input control points
Curve evalBezier( const vector< Vector3f >& P, unsigned steps ) {

	/* 
	Description:
		Function computes all the appropriate Vector3fs for each CurvePoint: V,T,N,B.
		(Assume all Bezier curves received have G1 continuity. TNB will not be be defined otherwise.)
		
	Variables:
		P: vector of points.
		steps:  the number of points to generate on each piece of the spline.

	Output:
		returns a Curve (e.g., a vector< CurvePoint >).
	*/

    // error checks for vector size (homogeneous coordinate)
    if( P.size() < 4 || P.size() % 3 != 1 )
    {
        cerr << "evalBezier must be called with 3n+1 control points." << endl;
        exit( 0 );
    }

	// printing input points
    cerr << "\t>>> evalBezier has been called with the following input:" << endl;
    cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
    for( unsigned i = 0; i < P.size(); ++i ){ cerr << "\t>>> " << P[i] << endl; }
    cerr << "\t>>> Steps (type steps): " << steps << endl;
    cerr << "\t>>> Returning empty curve." << endl;

	Curve curve; // declaring curve struct
	CurvePoint point, new_point; // declaring curve points 

	// Bezier basis
	Matrix4f Bezier_basis = Matrix4f(1., -3., 3., -1.,
		0., 3., -6., 3.,
		0., 0., 3., -3.,
		0., 0., 0., 1.);

	float dt = 1. / steps;
	Vector4f x, y, z, basis, basis_prime; // points
	Vector3f V, B, N, T; // vectors

	// loop over control points
	for (unsigned i = 3; i < P.size(); i = i + 1) {

		// computing vertex entries for current iteration
		x = Vector4f(P[i - 3][0], P[i - 2][0], P[i - 1][0], P[i][0]);
		y = Vector4f(P[i - 3][1], P[i - 2][1], P[i - 1][1], P[i][1]);
		z = Vector4f(P[i - 3][2], P[i - 2][2], P[i - 1][2], P[i][2]);

		// looking at last point of current segment and first point of next segment
		if (i == 3) {
			B = Vector3f(0., 0., 1.);
			T = new_coordinates(Bezier_basis * mono_basis(0), x, y, z);

			// checking for T != B
			if (Vector3f::cross(B, T) == Vector3f::ZERO) {
				B = Vector3f(0., 1., 0.);
			}
		}

		// loop over discretized parameter values
		for (unsigned t_i = 0; t_i <= steps; t_i++) {
			basis = Bezier_basis * mono_basis(t_i * dt); // Bezier basis
			basis_prime = Bezier_basis * mono_basis_prime(t_i * dt); // Bezier differentiated basis

			// computing Frenet-Serret vectors
			V = new_coordinates(basis, x, y, z);
			T = new_coordinates(basis_prime, x, y, z);
			N = Vector3f::cross(B, T);
			B = Vector3f::cross(T, N);

			// normalizing vectors
			T.normalize();
			N.normalize();
			B.normalize();

			// updating vectors for next iteration
			new_point = { V, T, N, B };
			if (t_i == 0 && i == 3) {
				point = new_point;
			}
			curve.push_back(new_point); // appending vectors to struct
		}
	}

	return curve; // returns curve struct
}


// function that generates points of B-spline curve given input control points
Curve evalBspline( const vector< Vector3f >& P, unsigned steps )
{
	/*
	Description:
		Function computes all the appropriate Vector3fs for each CurvePoint for a B-spline: V,T,N,B.

	Variables:
		P: vector of control points.
		steps: the number of points to generate on each piece of the spline.

	Output:
		returns a Curve (e.g., a vector< CurvePoint >).
	*/

	// Check
    if( P.size() < 4 )
    {
        cerr << "evalBspline must be called with 4 or more control points." << endl;
        exit( 0 );
    }

    cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

    cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
    for( unsigned i = 0; i < P.size(); ++i )
    {
        cerr << "\t>>> " << P[i] << endl;
    }

    cerr << "\t>>> Steps (type steps): " << steps << endl;
    cerr << "\t>>> Returning empty curve." << endl;

	Curve curve;
	CurvePoint point, new_point;

	// defining the B-spline basis as a matrix
	Matrix4f Bspline_basis;
	Bspline_basis = Matrix4f(1./6., -0.5, 0.5, -1. / 6.,
		2./3., 0, -1., 0.5,
		1. / 6., 0.5, 0.5, -0.5,
		0, 0, 0, 1. / 6.);
	
	float dt = 1. / steps;
	Vector4f x, y, z, basis, basis_prime; // points
	Vector3f V, B, N, T; // vectors
	
	// loop over control points
	for (unsigned i = 3; i < P.size(); i = i + 1) {

		// computing vertex entries for current iteration
		x = Vector4f(P[i - 3][0], P[i - 2][0], P[i - 1][0], P[i][0]);
		y = Vector4f(P[i - 3][1], P[i - 2][1], P[i - 1][1], P[i][1]);
		z = Vector4f(P[i - 3][2], P[i - 2][2], P[i - 1][2], P[i][2]);

		// looking at last point of current segment and first point of next segment
		if (i == 3) {
			B = Vector3f(0., 0., 1.);
			T = new_coordinates(Bspline_basis * mono_basis(0), x, y, z);

			// checking for T != B
			if (Vector3f::cross(B, T) == Vector3f::ZERO) {
				B = Vector3f(0., 1., 0.);
			}
		}

		// loop over discretized parameter values
		for (unsigned t_i = 0; t_i <= steps; t_i++) {
			basis = Bspline_basis * mono_basis(t_i * dt);
			basis_prime = Bspline_basis * mono_basis_prime(t_i * dt);
			
			// computing Frenet-Serret vectors
			V = new_coordinates(basis, x, y, z);
			T = new_coordinates(basis_prime, x, y, z);
			N = Vector3f::cross(B, T);
			B = Vector3f::cross(T, N);

			// normalizing vectors
			T.normalize();
			N.normalize();
			B.normalize();

			// updating vectors for next iteration
			new_point = { V, T, N, B };
			if (t_i == 0 && i == 3) {
				point = new_point;
			}
			curve.push_back(new_point); // appending vectors to struct
		}
	}

	return curve; // returns curve struct
}

Curve evalCircle( float radius, unsigned steps )
{
    // This is a sample function on how to properly initialize a Curve
    // (which is a vector< CurvePoint >).
    
    // Preallocate a curve with steps+1 CurvePoints
    Curve R( steps+1 );

    // Fill it in counterclockwise
    for( unsigned i = 0; i <= steps; ++i )
    {
        // step from 0 to 2pi
        float t = 2.0f * M_PI * float( i ) / steps;

        // Initialize position
        // We're pivoting counterclockwise around the y-axis
        R[i].V = radius * Vector3f( cos(t), sin(t), 0 );
        
        // Tangent vector is first derivative
        R[i].T = Vector3f( -sin(t), cos(t), 0 );
        
        // Normal vector is second derivative
        R[i].N = Vector3f( -cos(t), -sin(t), 0 );

        // Finally, binormal is facing up.
        R[i].B = Vector3f( 0, 0, 1 );
    }

    return R;
}

void drawCurve( const Curve& curve, float framesize )
{
    // Save current state of OpenGL
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Setup for line drawing
    glDisable( GL_LIGHTING ); 
    glColor4f( 1, 1, 1, 1 );
    glLineWidth( 1 );
    
    // Draw curve
    glBegin( GL_LINE_STRIP );
    for( unsigned i = 0; i < curve.size(); ++i )
    {
        glVertex( curve[ i ].V );
    }
    glEnd();

    glLineWidth( 1 );

    // Draw coordinate frames if framesize nonzero
    if( framesize != 0.0f )
    {
        Matrix4f M;

        for( unsigned i = 0; i < curve.size(); ++i )
        {
            M.setCol( 0, Vector4f( curve[i].N, 0 ) );
            M.setCol( 1, Vector4f( curve[i].B, 0 ) );
            M.setCol( 2, Vector4f( curve[i].T, 0 ) );
            M.setCol( 3, Vector4f( curve[i].V, 1 ) );

            glPushMatrix();
            glMultMatrixf( M );
            glScaled( framesize, framesize, framesize );
            glBegin( GL_LINES );
            glColor3f( 1, 0, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 1, 0, 0 );
            glColor3f( 0, 1, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 1, 0 );
            glColor3f( 0, 0, 1 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 0, 1 );
            glEnd();
            glPopMatrix();
        }
    }
    
    // Pop state
    glPopAttrib();
}

