
#include "MarlinTrk/HelixTrack.h"
#include <cmath>
#include <TVector3.h>
#include <kaltest/THelicalTrack.h>


HelixTrack::HelixTrack( const double* x1, const double* x2, const double* x3, double Bz, bool fitDirection ){

  // Make a KalTest THelicalTrack
  TVector3 p1( x1[0], x1[1], x1[2] );
  TVector3 p2( x2[0], x2[1], x2[2] );
  TVector3 p3( x3[0], x3[1], x3[2] );
  
  THelicalTrack helicalTrack( p1, p2, p3, Bz, fitDirection ); 

  
  // Set the track parameters and convert from the KalTest system to the lcio system
  
  _phi0 = toBaseRange( helicalTrack.GetPhi0() + M_PI/2. ) ;
  _omega = 1. / helicalTrack.GetRho();
  _z0 = helicalTrack.GetDz();
  _d0 = - helicalTrack.GetDrho();
  _tanLambda = helicalTrack.GetTanLambda();
  
  _ref_point_x =  helicalTrack.GetPivot().X() ;
  _ref_point_y =  helicalTrack.GetPivot().Y() ;
  _ref_point_z =  helicalTrack.GetPivot().Z() ;
  
 
  
  
}
  
  
HelixTrack::HelixTrack( const double* position, const double* p, double charge, double Bz ){
  
  _ref_point_x = position[0] ;
  _ref_point_y = position[1] ;
  _ref_point_z = position[2] ;
  
  _d0 = 0.0 ;
  _z0 = 0.0 ;
  
  const double pt = sqrt(p[0]*p[0]+p[1]*p[1]) ;
  
  double radius = pt / (2.99792458E-4*Bz) ; // for r in mm, p in GeV and Bz in Tesla
  
  _omega = charge/radius ;
  _tanLambda = p[2]/pt ;
  
  _phi0 = atan2(p[1],p[0]);
  
  while ( _phi0 < -M_PI ) _phi0 += 2.0*M_PI ;
  while ( _phi0 >= M_PI ) _phi0 -= 2.0*M_PI;

}

void HelixTrack::moveRefPoint( double x, double y, double z){
  
  const double radius = 1.0/_omega ; 
  
  const double sinPhi0 = sin(_phi0) ;
  const double cosPhi0 = cos(_phi0) ;
  
  const double deltaX = x - _ref_point_x ;
  const double deltaY = y - _ref_point_y ;
  
  double phi0Prime = atan2( sinPhi0 - (deltaX/(radius-_d0)) , cosPhi0 + (deltaY/(radius-_d0)) ) ;
  
  while ( phi0Prime < 0 )  phi0Prime += 2.0*M_PI ;
  while ( phi0Prime >= 2.0*M_PI ) phi0Prime -= 2.0*M_PI ;
  
  const double d0Prime = _d0 + deltaX*sinPhi0 - deltaY*cosPhi0 + ( ( deltaX*cosPhi0 + deltaY*sinPhi0 ) * tan( (phi0Prime-_phi0) / 2.0) ) ;
  
  // In order to have terms which behave well as Omega->0 we make use of deltaX and deltaY to replace sin( phi0Prime - phi0 ) and cos( phi0Prime - phi0 )
  
  const double sinDeltaPhi = ( -_omega / ( 1.0 - ( _omega * d0Prime ) ) ) * ( deltaX * cosPhi0 + deltaY * sinPhi0 ) ;
  
  const double cosDeltaPhi  = 1.0 + ( _omega*_omega / ( 2.0 * ( 1.0 - _omega * d0Prime ) ) ) * ( d0Prime*d0Prime - ( deltaX + _d0 * sinPhi0 )*( deltaX + _d0 * sinPhi0 ) - ( deltaY - _d0 * cosPhi0 )*( deltaY - _d0 * cosPhi0 ) ) ;
  
  const double s = atan2(-sinDeltaPhi,cosDeltaPhi) / _omega ;
  
  const double z0Prime  = _ref_point_z - z + _z0 + _tanLambda * s ;
  
  _d0   = d0Prime ;
  _phi0 = phi0Prime ;
  _z0   = z0Prime ;
  
  _ref_point_x = x; 
  _ref_point_y = y;
  _ref_point_z = z;   
  
}





