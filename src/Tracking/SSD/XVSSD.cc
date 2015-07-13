// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVSSD.cc
 *
 * @author Gregory D. Hager, Sam Lang
 * @version $Id: XVSSD.cc,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 *
 */

#include "XVSSDHelper.h"

#define _XVAffineSTATE_OP_EQ_(_OP_) \
	XVAffineState & XVAffineState::operator _OP_ (const XVAffineState & addon) { \
	this->trans _OP_ addon.trans; \
	this->a _OP_ addon.a; \
	this->b _OP_ addon.b; \
	this->c _OP_ addon.c; \
	this->d _OP_ addon.d; \
	return *this; \
};

_XVAffineSTATE_OP_EQ_( += );
_XVAffineSTATE_OP_EQ_( -= );


#define _XVAffineSTATE_OP_(_OP_) \
	XVAffineState operator _OP_ (const XVAffineState & s1, const XVAffineState & s2) { \
	\
	XVAffineState res; \
	res.trans = s1.trans _OP_ s2.trans; \
	res.a = s1.a _OP_ s2.a; \
	res.b = s1.b _OP_ s2.b; \
	res.c = s1.c _OP_ s2.c; \
	res.d = s1.d _OP_ s2.d; \
	return res; \
};

_XVAffineSTATE_OP_(+);
_XVAffineSTATE_OP_(-);
#define _XVSE2STATE_OP_EQ_(_OP_) \
XVSE2State & XVSE2State::operator _OP_ (const XVSE2State & addon) { \
  this->trans _OP_ addon.trans; \
  this->angle _OP_ addon.angle; \
  this->scale _OP_ addon.scale; \
  return *this; \
};

_XVSE2STATE_OP_EQ_( += );
_XVSE2STATE_OP_EQ_( -= );


#define _XVSE2STATE_OP_(_OP_) \
XVSE2State operator _OP_ (const XVSE2State & s1, const XVSE2State & s2) { \
\
  XVSE2State res; \
  res.trans = s1.trans _OP_ s2.trans; \
  res.angle = s1.angle _OP_ s2.angle; \
  res.scale = s1.scale _OP_ s2.scale; \
  return res; \
};

_XVSE2STATE_OP_(+);
_XVSE2STATE_OP_(-);

#define _XVRTSTATE_OP_EQ_(_OP_) \
XVRTState & XVRTState::operator _OP_ (const XVRTState & addon) { \
  this->trans _OP_ addon.trans; \
  this->angle _OP_ addon.angle; \
  return *this; \
};

_XVRTSTATE_OP_EQ_( += );
_XVRTSTATE_OP_EQ_( -= );

#define _XVRTSTATE_OP_(_OP_) \
XVRTState operator _OP_ (const XVRTState & s1, const XVRTState & s2) { \
\
  XVRTState res; \
  res.trans = s1.trans _OP_ s2.trans; \
  res.angle = s1.angle _OP_ s2.angle; \
  return res; \
};

_XVRTSTATE_OP_(+);
_XVRTSTATE_OP_(-);


#define _XVRotateSTATE_OP_EQ_(_OP_) \
XVRotateState & XVRotateState::operator _OP_ (const XVRotateState & addon) { \
  this->angle _OP_ addon.angle; \
  return *this; \
};

_XVRotateSTATE_OP_EQ_( += );
_XVRotateSTATE_OP_EQ_( -= );

#define _XVRotateSTATE_OP_(_OP_) \
XVRotateState operator _OP_ (const XVRotateState & s1, const XVRotateState & s2) { \
\
  XVRotateState res; \
  res.trans = s1.trans; \
  res.angle = s1.angle _OP_ s2.angle; \
  return res; \
};

_XVRotateSTATE_OP_(+);
_XVRotateSTATE_OP_(-);

#define _XVScalingSTATE_OP_EQ_(_OP_) \
XVScalingState & XVScalingState::operator _OP_ (const XVScalingState & addon) { \
  this->scale _OP_ addon.scale; \
  return *this; \
};

_XVScalingSTATE_OP_EQ_( += );
_XVScalingSTATE_OP_EQ_( -= );

#define _XVScalingSTATE_OP_(_OP_) \
XVScalingState operator _OP_ (const XVScalingState & s1, const XVScalingState & s2) { \
\
  XVScalingState res; \
  res.trans = s1.trans; \
  res.angle = s1.angle; \
  res.scale = s1.scale _OP_ s2.scale; \
  return res; \
};

_XVScalingSTATE_OP_(+);
_XVScalingSTATE_OP_(-);

/*********************************Misc Functions*********************************/

static void
check_brightness(XVImageScalar<float> &im) {
    XVImageWIterator<float> iter(im);
    for(; !iter.end(); ++iter) {
        if(*iter > 200) *iter = 0;
    }
}

#define OUTLIER_THRESH 35
static void
check_outliers(XVColVector &diff_vec, double & err) {
    int         turnoffs = 0;

    for(int i = 0; i < diff_vec.n_of_rows(); i++) {
        if(fabs(diff_vec[i]) > OUTLIER_THRESH) {
            diff_vec[i] = OUTLIER_THRESH / fabs(diff_vec[i]) * diff_vec[i];
            turnoffs++;
        }
    }
    err = (double) turnoffs / diff_vec.n_of_rows();
}

/*********************************XVSSDStepper*********************************/


template <class IM_TYPE, class ST_TYPE>
XVSSDStepper<IM_TYPE, ST_TYPE >::XVSSDStepper(const IM_TYPE & itemplate_in) {

    XVImageScalar<float> template_in;
    RGBtoScalar(itemplate_in, template_in);
    size = (XVSize) template_in;

    Dx = XVBoxFilterY(XVPrewittFilterX(template_in, (float) 0),
                      3, (float) 6);
    Dy = XVBoxFilterX(XVPrewittFilterY(template_in, (float) 0),
                      3, (float) 6);

    target = XVBoxFilter(template_in - template_in.avg(), 3, 3);
    forward_model.resize(target.Width() *target.Height(), 0);

    X = XVImageScalar<float> (target.Width(), target.Height());
    Y = XVImageScalar<float> (target.Width(), target.Height());

    XVImageWIterator<float > Xiter(X);
    XVImageWIterator<float > Yiter(Y);

    for(; !Xiter.end(); ++Xiter, ++Yiter) {

        *Xiter = (float) Xiter.currposx();
        *Yiter = (float) Yiter.currposy();
    }
};

/*********************************XVAffineStepper*********************************/

template <class IM_TYPE>
void XVAffineStepper<IM_TYPE >::offlineInit() {

    forward_model = add_column(forward_model, Dx);
    forward_model = add_column(forward_model, Dy);

    XVImageScalar<float> XDx = (X * Dx) ;
    XVImageScalar<float> XDy = (X * Dy) ;
    XVImageScalar<float> YDx = (Y * Dx) ;
    XVImageScalar<float> YDy = (Y * Dy) ;

    forward_model = add_column(forward_model, XDx);
    forward_model = add_column(forward_model, XDy);
    forward_model = add_column(forward_model, YDx);
    forward_model = add_column(forward_model, YDy);

    forward_model = add_column(forward_model, target);

    inverse_model = (((forward_model.t() * forward_model).i()) * forward_model.t());
};
template <class IM_TYPE>
XVStatePair<XVAffineState, double>
XVAffineStepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
                                const XVAffineState & oldState) {

    XVColVector result(7);
    XVMatrix sigma(7, 7);
    double error = 0.0;

    diff_intensity << (XVBoxFilter(live_image - live_image.avg(), 3, 3) - target);
    check_outliers(diff_intensity, error);

    sigma = 0;
    for(int i = 0; i <= 2; i++) {
        sigma[2 * i][2 * i] = oldState.a;
        sigma[2 * i + 1][2 * i] = oldState.b;
        sigma[2 * i][2 * i + 1] = oldState.c;
        sigma[2 * i + 1][2 * i + 1] = oldState.d;
    }
    sigma[6][6] = 1;
    result = sigma.t() * (inverse_model * diff_intensity);

    //std::cout<<"sigma:\n"<<sigma<<"\n";
    //std::cout<<"result:\n"<<result<<"\n";

    XVAffineState deltaMu;
    deltaMu.trans = XV2Vec<double> (-result[0], -result[1]);
    deltaMu.a = -result[2];
    deltaMu.b = -result[3];
    deltaMu.c = -result[4];
    deltaMu.d = -result[5];

    XVStatePair<XVAffineState, double> ret(deltaMu, error);

    return ret;
};


template <class IM_TYPE>
XVImageScalar<float> XVAffineStepper<IM_TYPE >::warp(const IM_TYPE    & image,
        const XVAffineState & state) {

    XVImageScalar<float> tmp;
    XVAffineMatrix warp_matrix(state.a, state.b, state.c, state.d);
    IM_TYPE warped_image = warpRect(image, (XVPosition) state.trans, size, warp_matrix);
    RGBtoScalar(warped_image, tmp);

    return (tmp);
};


/*********************************XVSE2Stepper*********************************/


template <class IM_TYPE>
void XVSE2Stepper<IM_TYPE >::offlineInit() {

    forward_model = add_column(forward_model, Dx);
    forward_model = add_column(forward_model, Dy);

    XVImageScalar<float> XDyYDx = (X * Dy) - (Y * Dx);
    XVImageScalar<float> XDxYDy = (X * Dx) + (Y * Dy);

    forward_model = add_column(forward_model, XDyYDx);
    forward_model = add_column(forward_model, XDxYDy);

    forward_model = add_column(forward_model, target);

    inverse_model = (((forward_model.t() * forward_model).i()) * forward_model.t());
};


template <class IM_TYPE>
XVStatePair<XVSE2State, double>
XVSE2Stepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
                             const XVSE2State & oldState) {

    XVColVector result(5);
    XVMatrix sigma(5, 5);
    double error = 0.0;

    diff_intensity << (XVBoxFilter(live_image - live_image.avg(), 3, 3) - target);
    check_outliers(diff_intensity, error);

    XVAffineMatrix rotMatrix(oldState.angle);
    sigma = 0;
    sigma[0][0] = rotMatrix(0, 0) / oldState.scale;
    sigma[1][0] = rotMatrix(1, 0);
    sigma[0][1] = rotMatrix(0, 1);
    sigma[1][1] = rotMatrix(1, 1) / oldState.scale;
    sigma[2][2] = 1;
    sigma[3][3] = 1 / oldState.scale;
    sigma[4][4] = 1;
    result = sigma.t() * (inverse_model * diff_intensity);

    XVSE2State deltaMu;
    deltaMu.trans = XV2Vec<double> (-result[0], -result[1]);
    deltaMu.angle = result[2];
    deltaMu.scale = result[3];

    XVStatePair<XVSE2State, double> ret(deltaMu, error);

    return ret;
};

template <class IM_TYPE>
XVImageScalar<float> XVSE2Stepper<IM_TYPE >::warp(const IM_TYPE    & image,
        const XVSE2State & state) {

    XVImageScalar<float> tmp;
    IM_TYPE warped_image = warpRect(image, (XVPosition) state.trans, size,
                                    state.angle, state.scale, state.scale, 0);
    RGBtoScalar(warped_image, tmp);

    return (tmp);
};

/*********************************XVRTStepper*********************************/


template <class IM_TYPE>
void XVRTStepper<IM_TYPE >::offlineInit() {
    forward_model = add_column(forward_model, Dx);
    forward_model = add_column(forward_model, Dy);

    XVImageScalar<float> XDyYDx = (X * Dy) - (Y * Dx);

    forward_model = add_column(forward_model, XDyYDx);
    forward_model = add_column(forward_model, target);
    inverse_model = (((forward_model.t() * forward_model).i()) * forward_model.t());

};

template <class IM_TYPE>
XVStatePair<XVRTState, double>
XVRTStepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
                            const XVRTState & oldState) {

    XVColVector result(4);
    XVMatrix sigma(4, 4);
    double error = 0.0;

    diff_intensity << (XVBoxFilter(live_image - live_image.avg(), 3, 3) - target);
    check_outliers(diff_intensity, error);

    XVAffineMatrix rotMatrix(oldState.angle);
    sigma = 0;
    sigma[0][0] = rotMatrix(0, 0);
    sigma[1][0] = rotMatrix(1, 0);
    sigma[0][1] = rotMatrix(0, 1);
    sigma[1][1] = rotMatrix(1, 1);
    sigma[2][2] = 1;
    sigma[3][3] = 1;
    result = sigma.t() * (inverse_model * diff_intensity);

    XVRTState deltaMu;
    deltaMu.trans = XV2Vec<double> (-result[0], -result[1]);
    deltaMu.angle = result[2];

    XVStatePair<XVRTState, double> ret(deltaMu, error);

    return ret;

};

template <class IM_TYPE>
XVImageScalar<float> XVRTStepper<IM_TYPE >::warp(const IM_TYPE    & image,
        const XVRTState & state) {

    XVImageScalar<float> tmp;
    IM_TYPE warped_image = warpRect(image, (XVPosition) state.trans, size,
                                    state.angle);
    RGBtoScalar(warped_image, tmp);

    return (tmp);
};

/*********************************XVScalingStepper*********************************/


template <class IM_TYPE>
void XVScalingStepper<IM_TYPE >::offlineInit() {
    //XVImageScalar<float> XDx = X * Dx;
    //XVImageScalar<float> YDy = Y * Dy;

    XVImageScalar<float> XDxYDy = (X * Dx) + (Y * Dy);

    forward_model = add_column(forward_model, XDxYDy);
    inverse_model = (((forward_model.t() * forward_model).i()) * forward_model.t());

    //std::cout<<"forward_model:\n"<<forward_model<<"\n";
    //std::cout<<"inverse_model:\n"<<inverse_model<<"\n";
};

template <class IM_TYPE>
XVStatePair<XVScalingState, double>
XVScalingStepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
                                 const XVScalingState & oldState) {

    XVColVector result(1);
    XVMatrix sigma(1, 1);
    double error = 0.0;

    diff_intensity << (XVBoxFilter(live_image - live_image.avg(), 3, 3) - target);
    check_outliers(diff_intensity, error);

    result = oldState.scale * (inverse_model * diff_intensity);
    //std::cout<<"diff_intensity:\n"<<diff_intensity<<"\n";
    //std::cout<<"result: "<<result<<"\n";

    XVScalingState deltaMu;
    deltaMu.scale = result[0];
    deltaMu.trans = XV2Vec<double> (0, 0);
    deltaMu.angle = 0;

    XVStatePair<XVScalingState, double> ret(deltaMu, error);

    return ret;
};

template <class IM_TYPE>
XVImageScalar<float> XVScalingStepper<IM_TYPE >::warp(const IM_TYPE    & image,
        const XVScalingState & state) {

    XVImageScalar<float> tmp;
    IM_TYPE warped_image = warpRect(image, (XVPosition) state.trans, size,
                                    state.angle, state.scale, state.scale, 0);
    RGBtoScalar(warped_image, tmp);

    return (tmp);

};


/*********************************XVRotateStepper*********************************/



template <class IM_TYPE>
void XVRotateStepper<IM_TYPE >::offlineInit() {

    //copy from XVRT
    forward_model = add_column(forward_model, Dx);
    forward_model = add_column(forward_model, Dy);
    XVImageScalar<float> XDyYDx = (X * Dy) - (Y * Dx);
    forward_model = add_column(forward_model, XDyYDx);
    forward_model = add_column(forward_model, target);
    inverse_model = (((forward_model.t() * forward_model).i()) * forward_model.t());

};

template <class IM_TYPE>
XVStatePair<XVRotateState, double>
XVRotateStepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
                                const XVRotateState & oldState) {

    XVColVector result(4);
    XVMatrix sigma(4, 4);
    double error = 0.0;

    diff_intensity << (XVBoxFilter(live_image - live_image.avg(), 3, 3) - target);
    check_outliers(diff_intensity, error);

    XVAffineMatrix rotMatrix(oldState.angle);
    sigma = 0;
    sigma[0][0] = rotMatrix(0, 0);
    sigma[1][0] = rotMatrix(1, 0);
    sigma[0][1] = rotMatrix(0, 1);
    sigma[1][1] = rotMatrix(1, 1);
    sigma[2][2] = 1;
    sigma[3][3] = 1;
    result = sigma.t() * (inverse_model * diff_intensity);

    XVRotateState deltaMu;
    deltaMu.trans = XV2Vec<double> (-result[0], -result[1]);
    deltaMu.angle = result[2];

    XVStatePair<XVRotateState, double> ret(deltaMu, error);

    return ret;
};

template <class IM_TYPE>
XVImageScalar<float> XVRotateStepper<IM_TYPE >::warp(const IM_TYPE    & image,
        const XVRotateState & state) {

    XVImageScalar<float> tmp;
    IM_TYPE warped_image = warpRect(image, (XVPosition) state.trans, size,
                                    state.angle);
    RGBtoScalar(warped_image, tmp);

    return (tmp);

};


/*********************************XVTransStepper*********************************/


template <class IM_TYPE>
void XVTransStepper<IM_TYPE >::offlineInit() {

    forward_model = add_column(forward_model, target);
    forward_model = add_column(forward_model, Dx);
    forward_model = add_column(forward_model, Dy);
    inverse_model = ((forward_model.t() * forward_model).i()) * (forward_model.t());
};

template <class IM_TYPE>
typename XVTransStepper<IM_TYPE >::ResultPair
XVTransStepper<IM_TYPE >::step(const XVImageScalar<float>      & live_image,
                               const XVTransState & old_state) {

    XVColVector result(3);
    double error ;

    result[0]  = result[1] = result[2] = 0;
    diff_intensity << (XVBoxFilter(live_image - live_image.avg(), 3, 3) - target);

    result =  inverse_model * diff_intensity;

    XVTransState deltaMu(-result[1], -result[2]);
    error = (diff_intensity - (forward_model * result)).ip() / diff_intensity.n_of_rows();

    return ResultPair(deltaMu, error);
};

template <class IM_TYPE>
XVImageScalar<float> XVTransStepper<IM_TYPE >::warp(const IM_TYPE      & image,
        const XVTransState & state) {
    XVImageScalar<float> tmp;
    IM_TYPE warped_image = warpRect(image, (XVPosition) state, size, 0);

    RGBtoScalar(warped_image, tmp);
    return tmp;
};

/*********************************XVPyramidStepper*********************************/


template <class STEPPER_TYPE>
XVImageScalar<float> XVPyramidStepper<STEPPER_TYPE>::upperLayer
(const XVImageScalar<float>& lower) {
    // need a faster version of convolve-and-resample here -- Donald
    const int ksize = 5 ;
    static float kernel[ksize] = { 0.0614, 0.2448, 0.3877, 0.2448, 0.0614 } ;
    static XVImageScalar<float>
    kernelX(ksize, 1, new XVPixmap<float> (ksize, 1, kernel, false)),
            kernelY(1, ksize, new XVPixmap<float> (1, ksize, kernel, false)) ;

    XVImageScalar<float> temp1, mid, temp2, upper ;

    //convolve( lower, temp1, kernelX, 0 );
    XVBoxFilterX(lower, 3, 3.0f, temp1);
    resample(scale_factor, 1, temp1, mid);
    //convolve( mid, temp2, kernelY, 0 );
    XVBoxFilterY(mid, 3, 3.0f, temp2);
    resample(1, scale_factor, temp2, upper);

    return upper ;
}

template <class STEPPER_TYPE>
XVPyramidStepper<STEPPER_TYPE>::XVPyramidStepper
(const typename STEPPER_TYPE::IMAGE_TYPE& template_in, double scale,
 int levels) {
    // since the constructor of a stepper asks for a template image of IM_TYPE
    // instead of that of XVImageScalar<float> (and do the conversion internally)
    // here we have to implicitly convert to IM_TYPE needlessly
    scale_factor = scale ;
    steppers.push_back(STEPPER_TYPE(template_in));
    XVImageScalar<float> temp ;
    temp = (XVImageScalar<float>) template_in ;
    for(levels -- ; levels > 0 ; levels --) {
        temp = upperLayer(temp);
        steppers.push_back(STEPPER_TYPE((IM_TYPE) temp));
    }
    savedImage.resize(1, 1);    // let it alloc a dummy pixmap
}

template <class STEPPER_TYPE>
void XVPyramidStepper<STEPPER_TYPE>::offlineInit() {
    for(int i = 0 ; i < steppers.size() ; i ++) {
        steppers[i].offlineInit() ;
    }
}

namespace {

template<class ST_TYPE> void scaleUp(ST_TYPE& state, double x) {
    state *= x ;
}

template<> void scaleUp(XVRotateState& state, double x) {
    state.trans *= x ;
}

template<> void scaleUp(XVScalingState& state, double x) {
    state.trans *= x ;
}

template<> void scaleUp(XVRTState& state, double x) {
    state.trans *= x ;
}

template<> void scaleUp(XVSE2State& state, double x) {
    state.trans *= x ;
}

template<> void scaleUp(XVAffineState& state, double x) {
    state.trans *= x ;
}

}

template <class STEPPER_TYPE>
XVStatePair<typename STEPPER_TYPE::STATE_TYPE, double>
XVPyramidStepper<STEPPER_TYPE>::step
(const XVImageScalar<float>& live_image,
 const typename STEPPER_TYPE::STATE_TYPE& old_state) {
    // a hack here -- assuming the same image as saved in warp()
    int i, j ;
    ST_TYPE new_state = old_state ;
    ResultPair delta_state ;
    XVImageScalar<float> currentImage = live_image ;

    for(i = steppers.size() - 1 ; i >= 0 ; i --) {
        if(i != steppers.size() - 1) {
            currentImage = steppers[0].warp(savedImage, new_state);
        }
        for(j = 0 ; j < i ; j ++) {
            currentImage = upperLayer(currentImage);
        }

        delta_state = steppers[i].step(currentImage, new_state);

        for(j = 0 ; j < i ; j ++) {
            scaleUp(delta_state.state, 1 / scale_factor);
        }
        new_state += delta_state.state ;
    }
    delta_state.state = new_state - old_state ;
    return delta_state ;
}

template <class STEPPER_TYPE>
XVImageScalar<float> XVPyramidStepper<STEPPER_TYPE>::warp
(const typename STEPPER_TYPE::IMAGE_TYPE& image,
 const typename STEPPER_TYPE::STATE_TYPE& state) {
    savedImage = image ; // a hack -- see step()
    return steppers[0].warp(image, state);
}

/*********************************XVSSD*********************************/


template <class IM_TYPE, class STEPPER_TYPE>
XVImageScalar<float>
XVSSD<IM_TYPE, STEPPER_TYPE>::warp(const IM_TYPE & image_in) {
    warped = Stepper.warp(image_in, currentState.state);
    warpUpdated = true;
    return warped;
};

template <class IM_TYPE, class STEPPER_TYPE>
const XVStatePair<typename STEPPER_TYPE::STATE_TYPE, double> &
XVSSD<IM_TYPE, STEPPER_TYPE>::step(const IM_TYPE & image_in) {

    if(!warpUpdated) this->warp(image_in);
    prevState = currentState;

    typename STEPPER_TYPE::ResultPair p;

    p = Stepper.step(warped, currentState.state);
    currentState.state = currentState.state + p.state;
    currentState.error = p.error;

    warpUpdated = false;
    return currentState;
};

/*********************************_REGISTER_STEPPERS_*********************************/

#include <XVFeature.h>

#define _REGISTER_STEPPERS_(_IM_TYPE_) \
template class XVTransStepper<_IM_TYPE_ >; \
template class XVRotateStepper<_IM_TYPE_ >; \
template class XVScalingStepper<_IM_TYPE_ >; \
template class XVRTStepper<_IM_TYPE_ >; \
template class XVAffineStepper<_IM_TYPE_ >; \
template class XVSE2Stepper<_IM_TYPE_ >;

template class XVPyramidStepper<XVTransStepper<XVImageRGB<XV_RGB24> > >;
template class XVPyramidStepper<XVTransStepper<XVImageRGB<XV_RGBA32> > >;

template class XVPyramidStepper<XVRotateStepper<XVImageRGB<XV_RGB24> > >;
template class XVPyramidStepper<XVRotateStepper<XVImageRGB<XV_RGBA32> > >;

template class XVPyramidStepper<XVScalingStepper<XVImageRGB<XV_RGB24> > >;
template class XVPyramidStepper<XVScalingStepper<XVImageRGB<XV_RGBA32> > >;

template class XVPyramidStepper<XVRTStepper<XVImageRGB<XV_RGB24> > >;
template class XVPyramidStepper<XVRTStepper<XVImageRGB<XV_RGBA32> > >;

template class XVPyramidStepper<XVAffineStepper<XVImageRGB<XV_RGB24> > >;
template class XVPyramidStepper<XVAffineStepper<XVImageRGB<XV_RGBA32> > >;

template class XVPyramidStepper<XVSE2Stepper<XVImageRGB<XV_RGB24> > >;
template class XVPyramidStepper<XVSE2Stepper<XVImageRGB<XV_RGBA32> > >;
//_REGISTER_STEPPERS_(XVImageScalar<int>);
//_REGISTER_STEPPERS_(XVImageScalar<float>);
//_REGISTER_STEPPERS_(XVImageScalar<double>);

//_REGISTER_STEPPERS_(XVImageScalar<unsigned char>);
_REGISTER_STEPPERS_(XVImageRGB<XV_RGB24>);
_REGISTER_STEPPERS_(XVImageRGB<XV_RGBA32>);

#define _SSD_PAIR_(_ST_TYPE_) XVStatePair<_ST_TYPE_, double>

#include <XVDrawable.h>

#define _REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, _IMAGE_TYPE_) \
template class XVSSD<_IMAGE_TYPE_, _STEPPER_TYPE_<_IMAGE_TYPE_ > >; \
template class XVSSD<_IMAGE_TYPE_, XVPyramidStepper<_STEPPER_TYPE_<_IMAGE_TYPE_ > > >;

/*
//_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageScalar<int>); \
//_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageScalar<float>); \
//_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageScalar<double>); \
*/

#define _REGISTER_XVSSD_SCALAR_(_STEPPER_TYPE_, _STATE_TYPE_) \
_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageRGB<XV_RGB24>);\
_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageRGB<XV_RGBA32>);

_REGISTER_XVSSD_SCALAR_(XVTransStepper, XVTransState);
_REGISTER_XVSSD_SCALAR_(XVRotateStepper,    XVRotateState);
_REGISTER_XVSSD_SCALAR_(XVScalingStepper,    XVScalingState);
_REGISTER_XVSSD_SCALAR_(XVRTStepper,    XVRTState);
_REGISTER_XVSSD_SCALAR_(XVSE2Stepper,   XVSE2State);
_REGISTER_XVSSD_SCALAR_(XVAffineStepper,   XVAffineState);


