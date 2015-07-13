#include "XVSSD.h"
#include "XVImageIterator.h"

/*********************************XVSSDHelper*********************************/

namespace {

template <class ST_TYPE>
struct XVSSDHelper {
};

/*********************************XVAffineState*********************************/


template<>
struct XVSSDHelper<XVAffineState> {
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVAffineState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, const IM_TYPE& im ) {
        XVPosition initULC;
        XVSize initSize;
        double initAngle;
        win.selectAngledRect(initULC, initSize, initAngle);
        XVAffineMatrix rotMat(initAngle);
        XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
                                    initSize.Height() / 2);
        currentState.state.trans = (initCenter + initULC);
        currentState.state.a = rotMat.a;
        currentState.state.b = rotMat.b;
        currentState.state.c = rotMat.c;
        currentState.state.d = rotMat.d;
        XVAffineMatrix warp_matrix(rotMat.a, rotMat.b, rotMat.c, rotMat.d);
        Stepper = STEPPER_TYPE(warpRect(im,
                                        currentState.state.trans,
                                        initSize,
                                        warp_matrix));

        Stepper.offlineInit();
        currentState.error = 0.0;
    }
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVAffineState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
        cerr << "not yet defined" << endl;
        exit(-1);
    }
    template<class STEPPER_TYPE>
    static void show
    ( XVStatePair<XVAffineState,double>& currentState, STEPPER_TYPE& Stepper,
      XVDrawable& x, float scale ) {
        XVPosition corners[4];
        XVAffineMatrix tformMat(currentState.state.a, currentState.state.b,
                                currentState.state.c, currentState.state.d);

        XV2Vec<double> points[4];
        XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2,
                                  Stepper.getSize().Height() / 2);
        points[0] = - tmpPoint;
        points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
        points[2] = tmpPoint;
        points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

        corners[0] = (tformMat * points[0]) + currentState.state.trans;
        corners[1] = (tformMat * points[1]) + currentState.state.trans;
        corners[2] = (tformMat * points[2]) + currentState.state.trans;
        corners[3] = (tformMat * points[3]) + currentState.state.trans;

        for(int i=0; i<4; i++) corners[i].setX((int)(corners[i].x()/scale)),
                corners[i].setY((int)(corners[i].y()/scale));
        x.drawLine(corners[0], corners[1]);
        x.drawLine(corners[1], corners[2]);
        x.drawLine(corners[2], corners[3]);
        x.drawLine(corners[3], corners[0]);
    }
};

/*********************************XVSE2State*********************************/


template<>
struct XVSSDHelper<XVSE2State> {
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVSE2State,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, const IM_TYPE& im ) {
        XVPosition initULC;
        XVSize initSize;
        double initAngle;
        win.selectAngledRect(initULC, initSize, initAngle);
        XVAffineMatrix rotMat(initAngle);
        XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
                                    initSize.Height() / 2);
        currentState.state.trans = (initCenter + initULC);
        currentState.state.angle = -initAngle;
        currentState.state.scale = 1;
        Stepper = STEPPER_TYPE(warpRect(im,
                                        currentState.state.trans,
                                        initSize,
                                        currentState.state.angle,
                                        currentState.state.scale,
                                        currentState.state.scale,
                                        0));
        Stepper.offlineInit();
        currentState.error = 0.0;
    }
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVSE2State,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
        cerr << "not yet defined" << endl;
        exit(-1);
    }
    template<class STEPPER_TYPE>
    static void show
    ( XVStatePair<XVSE2State,double>& currentState, STEPPER_TYPE& Stepper,
      XVDrawable& x, float scale ) {
        XVPosition corners[4];
        XVAffineMatrix angleMat(-currentState.state.angle);
        XVAffineMatrix scaleMat( 1 / currentState.state.scale,
                                 1 / currentState.state.scale);
        XVAffineMatrix tformMat((XVMatrix)scaleMat * (XVMatrix)angleMat);

        XV2Vec<double> points[4];
        XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2,
                                  Stepper.getSize().Height() / 2);
        points[0] = - tmpPoint;
        points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
        points[2] = tmpPoint;
        points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

        corners[0] = (tformMat * points[0]) + currentState.state.trans;
        corners[1] = (tformMat * points[1]) + currentState.state.trans;
        corners[2] = (tformMat * points[2]) + currentState.state.trans;
        corners[3] = (tformMat * points[3]) + currentState.state.trans;

        for(int i=0; i<4; i++) corners[i].setX((int)(corners[i].x()/scale)),
                corners[i].setY((int)(corners[i].y()/scale));
        x.drawLine(corners[0], corners[1]);
        x.drawLine(corners[1], corners[2]);
        x.drawLine(corners[2], corners[3]);
        x.drawLine(corners[3], corners[0]);
    }
};

/*********************************XVRTState*********************************/

template<>
struct XVSSDHelper<XVRTState> {
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVRTState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, const IM_TYPE& im ) {
        XVPosition initULC;
        XVSize initSize;
        double initAngle;
        win.selectAngledRect(initULC, initSize, initAngle);
        XVAffineMatrix rotMat(initAngle);
        XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
                                    initSize.Height() / 2);
        currentState.state.trans = (initCenter + initULC);
        currentState.state.angle = -initAngle;
        Stepper = STEPPER_TYPE(warpRect(im,
                                        currentState.state.trans,
                                        initSize,
                                        currentState.state.angle) );
        Stepper.offlineInit();
        currentState.error = 0.0;
    }
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVRTState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
        cerr << "not yet defined" << endl;
        exit(-1);
    }
    template<class STEPPER_TYPE>
    static void show
    ( XVStatePair<XVRTState,double>& currentState, STEPPER_TYPE& Stepper,
      XVDrawable& x, float scale ) {
        XVPosition corners[4];
        XVAffineMatrix angleMat(-currentState.state.angle);
        XVAffineMatrix tformMat((XVMatrix)angleMat);

        XV2Vec<double> points[4];
        XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2,
                                  Stepper.getSize().Height() / 2);
        points[0] = - tmpPoint;
        points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
        points[2] = tmpPoint;
        points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

        corners[0] = (tformMat * points[0]) + currentState.state.trans;
        corners[1] = (tformMat * points[1]) + currentState.state.trans;
        corners[2] = (tformMat * points[2]) + currentState.state.trans;
        corners[3] = (tformMat * points[3]) + currentState.state.trans;

        for(int i=0; i<4; i++) corners[i].setX((int)(corners[i].x()/scale)),
                corners[i].setY((int)(corners[i].y()/scale));
        x.drawLine(corners[0], corners[1]);
        x.drawLine(corners[1], corners[2]);
        x.drawLine(corners[2], corners[3]);
        x.drawLine(corners[3], corners[0]);
    }
};

/*********************************XVScalingState*********************************/


template<>
struct XVSSDHelper<XVScalingState> {
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVScalingState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, const IM_TYPE& im ) {
        XVPosition initULC;
        XVSize initSize;
        double initAngle;
        win.selectAngledRect(initULC, initSize, initAngle);
        XVAffineMatrix rotMat(initAngle);
        XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
                                    initSize.Height() / 2);
        currentState.state.trans = (initCenter + initULC);
        currentState.state.angle = -initAngle;
        currentState.state.scale = 1;
        Stepper = STEPPER_TYPE(warpRect(im,
                                        currentState.state.trans,
                                        initSize,
                                        currentState.state.angle,
                                        currentState.state.scale,
                                        currentState.state.scale,
                                        0));
        Stepper.offlineInit();
        currentState.error = 0.0;
    }
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVScalingState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
        cerr << "not yet defined" << endl;
        exit(-1);
    }
    template<class STEPPER_TYPE>
    static void show
    ( XVStatePair<XVScalingState,double>& currentState, STEPPER_TYPE& Stepper,
      XVDrawable& x, float scale ) {
        XVPosition corners[4];
        XVAffineMatrix angleMat(-currentState.state.angle);
        XVAffineMatrix scaleMat( 1 / currentState.state.scale,
                                 1 / currentState.state.scale);
        XVAffineMatrix tformMat((XVMatrix)scaleMat * (XVMatrix)angleMat);

        XV2Vec<double> points[4];
        XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2,
                                  Stepper.getSize().Height() / 2);
        points[0] = - tmpPoint;
        points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
        points[2] = tmpPoint;
        points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

        corners[0] = (tformMat * points[0]) + currentState.state.trans;
        corners[1] = (tformMat * points[1]) + currentState.state.trans;
        corners[2] = (tformMat * points[2]) + currentState.state.trans;
        corners[3] = (tformMat * points[3]) + currentState.state.trans;

        for(int i=0; i<4; i++) corners[i].setX((int)(corners[i].x()/scale)),
                corners[i].setY((int)(corners[i].y()/scale));
        x.drawLine(corners[0], corners[1]);
        x.drawLine(corners[1], corners[2]);
        x.drawLine(corners[2], corners[3]);
        x.drawLine(corners[3], corners[0]);
    }
};

/*********************************XVRotateState*********************************/

template<>
struct XVSSDHelper<XVRotateState> {
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVRotateState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, const IM_TYPE& im ) {
        XVPosition initULC;
        XVSize initSize;
        double initAngle;
        win.selectAngledRect(initULC, initSize, initAngle);
        XVAffineMatrix rotMat(initAngle);
        XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
                                    initSize.Height() / 2);
        currentState.state.trans= (initCenter + initULC);
        currentState.state.angle= -initAngle;
        Stepper = STEPPER_TYPE( warpRect(im,
                                         currentState.state.trans,
                                         initSize,
                                         currentState.state.angle) );
        Stepper.offlineInit();
        currentState.error = 0.0;
    }
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVRotateState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
        cerr << "not yet defined" << endl;
        exit(-1);
    }
    template<class STEPPER_TYPE>
    static void show
    ( XVStatePair<XVRotateState,double>& currentState, STEPPER_TYPE& Stepper,
      XVDrawable& x, float scale ) {

        XVPosition corners[4];
        XVAffineMatrix angleMat(-currentState.state.angle);
        XVAffineMatrix tformMat((XVMatrix)angleMat);

        XV2Vec<double> points[4];
        XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2,
                                  Stepper.getSize().Height() / 2);
        points[0] = - tmpPoint;
        points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
        points[2] = tmpPoint;
        points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

        corners[0] = (tformMat * points[0]) + currentState.state.trans;
        corners[1] = (tformMat * points[1]) + currentState.state.trans;
        corners[2] = (tformMat * points[2]) + currentState.state.trans;
        corners[3] = (tformMat * points[3]) + currentState.state.trans;

        for(int i=0; i<4; i++) corners[i].setX((int)(corners[i].x()/scale)),
                corners[i].setY((int)(corners[i].y()/scale));
        x.drawLine(corners[0], corners[1]);
        x.drawLine(corners[1], corners[2]);
        x.drawLine(corners[2], corners[3]);
        x.drawLine(corners[3], corners[0]);

    }
};

/*********************************XVTransState*********************************/

template <>
struct XVSSDHelper<XVTransState> {
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVTransState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, const IM_TYPE& im ) {
        XVROI roi;
        win.selectRectangle(roi);
        IM_TYPE tmpl = subimage(im, roi);
        Stepper = STEPPER_TYPE(tmpl);
        Stepper.offlineInit();
        XV2Vec<double> center;
        center.setX(roi.Width()  );
        center.setY(roi.Height() );
        currentState.state = (XV2Vec<double>)roi + center;
        currentState.error = 0.0;
    }
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    ( XVStatePair<XVTransState,double>& currentState, STEPPER_TYPE& Stepper,
      XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
        XVROI roi;
        win.selectSizedRect(roi,size);
        IM_TYPE tmpl = subimage(im, roi);
        Stepper = STEPPER_TYPE(tmpl);
        Stepper.offlineInit();
        XV2Vec<double> center;
        center.setX(roi.Width()/2  );
        center.setY(roi.Height()/2 );
        currentState.state = (XV2Vec<double>)roi + center;
        currentState.error = 0.0;
    }
    template<class STEPPER_TYPE>
#ifndef NOVIS
    static void show
    ( XVStatePair<XVTransState,double>& currentState, STEPPER_TYPE& Stepper,
      XVDrawable& x,float scale ) {

        XVPosition corners[4];
        XV2Vec<double> points[4];
        XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2,
                                  Stepper.getSize().Height() / 2);

        points[0] = - tmpPoint;
        points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
        points[2] = tmpPoint;
        points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

        for(int i=0; i<4; ++i)
            corners[i] = points[i] + currentState.state;

        for(int i=0; i<4; i++) corners[i].setX((int)(corners[i].x()/scale)),
                corners[i].setY((int)(corners[i].y()/scale));

        x.drawLine(corners[0], corners[1]);
        x.drawLine(corners[1], corners[2]);
        x.drawLine(corners[2], corners[3]);
        x.drawLine(corners[3], corners[0]);
    }
};
#endif
}
/*********************************XVSSD*********************************/

template <class IM_TYPE, class STEPPER_TYPE>
const XVStatePair<typename STEPPER_TYPE::STATE_TYPE, double> &
XVSSD<IM_TYPE, STEPPER_TYPE>::interactiveInit
( XVInteractive & win, const IM_TYPE & im ) {
    XVSSDHelper<STATE_TYPE>::interactiveInit( currentState, Stepper, win, im );
    return prevState = currentState ;
}

template <class IM_TYPE, class STEPPER_TYPE>
const XVStatePair<typename STEPPER_TYPE::STATE_TYPE, double> &
XVSSD<IM_TYPE, STEPPER_TYPE>::interactiveInit
( XVInteractive & win, XVSize& size, const IM_TYPE & im ) {
    XVSSDHelper<STATE_TYPE>::interactiveInit( currentState, Stepper,win,size,im);
    return prevState = currentState ;
}

template <class IM_TYPE, class STEPPER_TYPE>
void XVSSD<IM_TYPE, STEPPER_TYPE>::show( XVDrawable& x ,float scale) {
    XVSSDHelper<STATE_TYPE>::show( currentState, Stepper, x,scale );
}
template<class IM_TYPE, class STEPPER_TYPE>
void XVSSD<IM_TYPE,STEPPER_TYPE>::setStepper(
    const STEPPER_TYPE& stepper_in) {
    Stepper = stepper_in ;
    Stepper.offlineInit();
}
