template<>
struct XVSSDHelper<XVState> {
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    (XVStatePair<XVState, double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, const IM_TYPE& im) {
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
                                        currentState.state.angle));
        Stepper.offlineInit();
        currentState.error = 0.0;
    }
    template<class IM_TYPE, class STEPPER_TYPE>
    static void interactiveInit
    (XVStatePair<XVState, double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, XVSize& size, const IM_TYPE& im) {
        cerr << "not yet defined" << endl;
        exit(-1);
    }
    template<class STEPPER_TYPE>
    static void show
    (XVStatePair<XVState, double>& currentState, STEPPER_TYPE& Stepper,
     XVDrawable& x, float scale) {
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

        for(int i = 0; i < 4; i++) corners[i].setX((int)(corners[i].x() / scale)),
                corners[i].setY((int)(corners[i].y() / scale));
        x.drawLine(corners[0], corners[1]);
        x.drawLine(corners[1], corners[2]);
        x.drawLine(corners[2], corners[3]);
        x.drawLine(corners[3], corners[0]);
    }
};
template <class IM_TYPE>
void XVStepper<IM_TYPE >::offlineInit() {
    forward_model = add_column(forward_model, Dx);
    forward_model = add_column(forward_model, Dy);
    XVImageScalar<float> XDyYDx = (X * Dy) - (Y * Dx);
    forward_model = add_column(forward_model, XDyYDx);
    forward_model = add_column(forward_model, target);
    inverse_model = (((forward_model.t() * forward_model).i()) * forward_model.t());
};

template <class IM_TYPE>
XVStatePair<XVState, double>
XVStepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
                          const XVState & oldState) {
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

    XVState deltaMu;
    deltaMu.trans = XV2Vec<double> (-result[0], -result[1]);
    deltaMu.angle = result[2];

    XVStatePair<XVState, double> ret(deltaMu, error);

    return ret;
};

template <class IM_TYPE>
XVImageScalar<float> XVStepper<IM_TYPE >::warp(const IM_TYPE    & image,
        const XVState & state) {

    XVImageScalar<float> tmp;
    IM_TYPE warped_image = warpRect(image, (XVPosition) state.trans, size,
                                    state.angle);
    RGBtoScalar(warped_image, tmp);

    return (tmp);
};
