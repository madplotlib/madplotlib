/* Copyright (C) 2017 Karl Phillip Buhr <karlphillip@gmail.com>
 *
 * This work is licensed under the MIT License.
 * To view a copy of this license, visit:
 *      https://opensource.org/licenses/MIT
 *
 * This file is part of Madplotlib, a C++ library for building simple
 * 2D plots inspired on matplotlib.
 */
#pragma once
#ifndef NO_EIGEN
#include <Eigen/Dense>
#endif

#include <QDebug>
#include <QEventLoop>
#include <QGraphicsLayout>
#include <QPair>

#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>

#pragma once

#ifndef PLT_ARG_NAMESPACE
#define PLT_ARG_NAMESPACE
#endif

#ifdef _MSC_VER
#if _MSC_VER >= 1900
#define PLT_CONSTEXPR constexpr
#else
#define PLT_CONSTEXPR
#endif
#else
#define PLT_CONSTEXPR constexpr
#endif

#define MO_KEYWORD_INPUT(name, type)                                                                                                  \
namespace tag{                                                                                                                        \
    struct name {                                                                                                                     \
        typedef type        Type;                                                                                                     \
        typedef const Type& ConstRef;                                                                                                 \
        typedef Type&       Ref;                                                                                                      \
        typedef ConstRef    StorageType;                                                                                              \
        typedef const void* VoidType;                                                                                                 \
        template <typename T>                                                                                                         \
        static PLT_CONSTEXPR bool AllowedType() { return std::is_same<Type, T>::value; }                                                  \
        static VoidType GetPtr(const Type& arg) {                                                                                     \
            return &arg;                                                                                                              \
        }                                                                                                                             \
        template <class T>                                                                                                            \
        static VoidType GetPtr(const T& arg) {                                                                                        \
            (void)arg;                                                                                                                \
            return nullptr;                                                                                                           \
        }                                                                                                                             \
    };                                                                                                                                \
}                                                                                                                                     \
namespace PLT_ARG_NAMESPACE {                                                                                                         \
    static kwargs::TKeyword<tag::name>& name = kwargs::TKeyword<tag::name>::instance;                                                 \
}


#define MO_KEYWORD_OUTPUT(name, type)                                                                                                 \
namespace tag{                                                                                                                        \
    struct name {                                                                                                                     \
        typedef type        Type;                                                                                                     \
        typedef const Type& ConstRef;                                                                                                 \
        typedef Type&       Ref;                                                                                                      \
        typedef Ref         StorageType;                                                                                              \
        typedef void*       VoidType;                                                                                                 \
        template <typename T>                                                                                                         \
        static PLT_CONSTEXPR bool AllowedType() { return std::is_same<Type, T>::value; }                                                  \
        static VoidType GetPtr(Type& arg) {                                                                                           \
            return &arg;                                                                                                              \
        }                                                                                                                             \
        template <class T>                                                                                                            \
        static VoidType GetPtr(const T& arg) {                                                                                        \
            (void)arg;                                                                                                                \
            return nullptr;                                                                                                           \
        }                                                                                                                             \
    };                                                                                                                                \
}                                                                                                                                     \
namespace PLT_ARG_NAMESPACE {                                                                                                         \
static kwargs::TKeyword<name>& name = kwargs::TKeyword<name>::instance;                                                               \
}

namespace kwargs {
    struct TaggedBase {};
    template <class Tag>
    struct TaggedArgument : public TaggedBase {
        typedef Tag TagType;
        explicit TaggedArgument(typename Tag::StorageType val)
            : arg(&val) {
        }

        typename Tag::VoidType get() const {
            return arg;
        }

    protected:
        typename Tag::VoidType arg;
    };

    template <class Tag>
    struct TKeyword {
        static TKeyword     instance;
        TaggedArgument<Tag> operator=(typename Tag::StorageType data) {
            return TaggedArgument<Tag>(data);
        }
    };
    template <class T>
    TKeyword<T> TKeyword<T>::instance;
}

template <int N, typename... T>
struct ArgType;

template <typename T0, typename... T>
struct ArgType<0, T0, T...> {
    typedef T0 type;
};
template <int N, typename T0, typename... T>
struct ArgType<N, T0, T...> {
    typedef typename ArgType<N - 1, T...>::type type;
};

template <class Tag, bool Infer = false>
typename Tag::VoidType GetKeyImpl() {
    return 0;
}

template <class T, class U>
PLT_CONSTEXPR int CountTypeImpl(const U& value) {
    return std::is_same<T, U>::value ? 1 : 0;
}

template <class T, class U, class... Args>
PLT_CONSTEXPR int CountTypeImpl(const U& value, const Args&... args) {
    return CountTypeImpl<T, Args...>(args...) + (std::is_same<T, U>::value ? 1 : 0);
}

template <class T, class... Args>
PLT_CONSTEXPR int CountType(const Args&... args) {
    return CountTypeImpl<T, Args...>(args...);
}

template <size_t N, typename... Args>
auto GetPositionalInput(Args&&... as) -> decltype(std::get<N>(std::forward_as_tuple(std::forward<Args>(as)...))) {
    return std::get<N>(std::forward_as_tuple(std::forward<Args>(as)...));
}

template <class Tag, bool Infer = false, class T, class... Args>
typename std::enable_if<std::is_base_of<kwargs::TaggedBase, T>::value, typename Tag::VoidType>::type
    GetKeyImpl(const T& arg, const Args&... args) {
    return std::is_same<typename T::TagType, Tag>::value ? const_cast<void*>(arg.get()) : const_cast<void*>(GetKeyImpl<Tag, Infer, Args...>(args...));
}

template <class Tag, bool Infer = false, class T, class... Args>
typename std::enable_if<!std::is_base_of<kwargs::TaggedBase, T>::value, typename Tag::VoidType>::type
    GetKeyImpl(const T& arg, const Args&... args) {
#ifdef __GNUC__
    //static_assert(CountType<typename Tag::Type>(arg, args...) <= 1, "Cannot infer type when there are multiple variadic Params with desired type");
#endif
    return Tag::template AllowedType<T>() && Infer ? // This infers the type
        Tag::GetPtr(arg)
        : const_cast<void*>(GetKeyImpl<Tag, Infer, Args...>(args...));
}

template <class Tag, bool Infer = false, class... Args>
typename Tag::ConstRef GetKeywordInput(const Args&... args) {
    const void* ptr = GetKeyImpl<Tag, Infer>(args...);
    assert(ptr);
    return *(static_cast<const typename Tag::Type*>(ptr));
}

template <class Tag, bool Infer = false, class... Args>
typename Tag::ConstRef GetKeywordInputDefault(typename Tag::ConstRef def, const Args&... args) {
    const void* ptr = GetKeyImpl<Tag, Infer>(args...);
    if (ptr)
        return *static_cast<const typename Tag::Type*>(ptr);
    return def;
}

template <class Tag, bool Infer = false, class... Args>
const typename Tag::Type* GetKeywordInputOptional(const Args&... args) {
    const void* ptr = GetKeyImpl<Tag, Infer>(args...);
    if (ptr)
        return static_cast<const typename Tag::Type*>(ptr);
    return nullptr;
}

template <class Tag, bool Infer = false, class... Args>
typename Tag::Ref GetKeywordOutput(const Args&... args) {
    static_assert(!std::is_const<typename Tag::VoidType>::value, "Tag type is not an output tag");
    void* ptr = GetKeyImpl<Tag, Infer>(args...);
    assert(ptr);
    return *(static_cast<typename Tag::Type*>(ptr));
}

template <class Tag, bool Infer = false, class... Args>
typename Tag::Type* GetKeywordOutputOptional(const Args&... args) {
    static_assert(!std::is_const<typename Tag::VoidType>::value, "Tag type is not an output tag");
    void* ptr = GetKeyImpl<Tag, Infer>(args...);
    if (ptr)
        return (static_cast<typename Tag::Type*>(ptr));
    return nullptr;
}

MO_KEYWORD_INPUT(x, Eigen::ArrayXf)
MO_KEYWORD_INPUT(y, Eigen::ArrayXf)
MO_KEYWORD_INPUT(marker, QString)
MO_KEYWORD_INPUT(label, QString)
MO_KEYWORD_INPUT(color, QColor)
MO_KEYWORD_INPUT(linewidth, quint32)
MO_KEYWORD_INPUT(alpha, qreal)
MO_KEYWORD_INPUT(edgecolor, QColor)
MO_KEYWORD_INPUT(markersize, qreal)

// The below works on MSVC 2015
/*template<typename T, typename Enable = void>
struct is_matrix_expression : std::false_type {};
template<typename T>
struct is_matrix_expression<T, decltype(std::declval<Eigen::ArrayXf>() = std::declval<T>(), void())> : std::true_type {};*/

// Not sure if there are limitations to this but it seems to work on 2013
template<typename Derived>
struct is_matrix_expression
	: std::is_base_of<Eigen::ArrayBase<std::decay_t<Derived> >, std::decay_t<Derived> >
{};



/* Debug control */

#define DEBUG               0      // level 0: debug msgs are disabled
                                   // level 1: print method calls
                                   // level 2: print method calls + data

/* Global definitions */

#define SHOW_TICK           1
#define HIDE_TICK           2
#define SHOW_CUSTOM_TICK    4

#define DEFAULT_LEGEND      ""
#define DEFAULT_MARKER      "-"
#define DEFAULT_ALPHA       1.0f
#define DEFAULT_COLOR       "none"
#define DEFAULT_EDGECOLOR   "none"
#define DEFAULT_LINEW       2
#define DEFAULT_MARKERSZ    6.0f

#ifdef NO_EIGEN
#error COMPILATION MUST GO THOUGH WITHOUT EIGEN CODE.
#endif


class Madplotlib
{
public:
    Madplotlib(bool isWidget = false)
    : _chart(NULL), _chartView(NULL), _isWidget(isWidget)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "Madplotlib(): isWidget=" << isWidget;
#endif
        _xAxisTop = _xAxisBottom = _yAxisLeft = _yAxisRight = nullptr;
        _chart = new QtCharts::QChart();

        _chartView = new QtCharts::QChartView(_chart);

        _enableGrid = false;
        _customLimits = false;
        _xMin = _xMax = _yMin = _yMax = 0;

        _showXticks = _showYticks = SHOW_TICK;
        _xTickCount = 7;
        _yTickCount = 5;

        _colorIdx = 0;
        _colors.push_back(QColor(0x1f77b4));
        _colors.push_back(QColor(0xff7f0e));
        _colors.push_back(QColor(0x2ca02c));
        _colors.push_back(QColor(0xd62728));
        _colors.push_back(QColor(0x9467bd));
        _colors.push_back(QColor(0x8c564b));
        _colors.push_back(QColor(0xe377c2));
        _colors.push_back(QColor(0x7f7f7f));
        _colors.push_back(QColor(0xbcbd22));
        _colors.push_back(QColor(0x17becf));
    }

    void axis(QString cmd)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "axis(): cmd=" << cmd;
#endif
        if (cmd == "off")
        {
            _showYticks = _showXticks = HIDE_TICK;
        }
        else if (cmd == "xoff")
        {
            _showXticks = HIDE_TICK;
        }
        else if (cmd == "yoff")
        {
            _showYticks = HIDE_TICK;
        }
        else
        {
            qCritical() << "axis()!!! options are 'off', 'xoff' and 'yoff'.";
            return;
        }
    }

    /* axis(): gets the current axes limits [xMin, xMax, yMin, yMax].
     */
    void axis(qreal* xMin, qreal* xMax, qreal* yMin, qreal* yMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "axis(): _xMin=" << _xMin << " _xMax=" << _xMax << " _yMin=" << _yMin << " _yMax=" << _yMax;
#endif
        *xMin = _xMin;
        *xMax = _xMax;
        *yMin = _yMin;
        *yMax = _yMax;
    }

    /* axis(): sets the viewport of the axis by a list of [xMin, xMax, yMin, yMax].
     */
    void axis(const qreal& xMin, qreal xMax, const qreal& yMin, const qreal& yMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "axis(): xMin=" << xMin << " xMax=" << xMax << " yMin=" << yMin << " yMax=" << yMax;
#endif
        _xMin = xMin;
        _xMax = xMax;
        _yMin = yMin;
        _yMax = yMax;
        _customLimits = true;
    }

    /* xlim(): sets the x limits of the current axes.
     */
    void xlim(const qreal& xMin, const qreal& xMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "xlim(): xMin=" << xMin << " xMax=" << xMax;
#endif
        _xMin = xMin;
        _xMax = xMax;
        _customLimits = true;
    }

    /* ylim(): sets the x limits of the current axes.
     */
    void ylim(const qreal& yMin, const qreal& yMax)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "ylim(): yMin=" << yMin << " yMax=" << yMax;
#endif
        _yMin = yMin;
        _yMax = yMax;
        _customLimits = true;
    }

    /* title(): defines the title of the chart.
     */
    void title(QString string)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "title(): string=" << string;
#endif
        _title = string;
    }

    /* xlabel(): defines the label displayed below the x axis.
     */
    void xlabel(QString label)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "xlabel(): label=" << label;
#endif
        _xLabel = label;
    }

    /* ylabel(): defines the label displayed to the left of the y axis.
     */
    void ylabel(QString label)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "ylabel(): label=" << label;
#endif
        _yLabel = label;
    }

    /* legend(): defines the position of the legend label inside the chart.
     */
    void legend()
    {
        _legendPos = "lower center";
    }

    /* legend(): defines the position of the legend label inside the chart.
     */
    void legend(QString cmd)
    {
        _legendPos = _parseLegendPos(cmd);
    }

    /* grid(): enables or disables the background grid of the chart.
     */
    void grid(bool status)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "grid(): status=" << status;
#endif
        _enableGrid = status;
    }

    /* savefig(): saves the chart displayed by show() as an image on the disk.
     */
    void savefig(QString filename)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "savefig(): filename=" << filename;
#endif
        if (!_pixmap.isNull())
            _pixmap.save(filename);
        else if(_isWidget && _chartView){
            _pixmap = _chartView->grab();
        }
    }

    /* xticks(): sets the x-limits of the current tick locations and labels.
     */
    void xticks(const Eigen::ArrayXf& values, const QVector<QString>& labels)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "xticks(): values.sz=" << values.rows() << " labels.sz=" << labels.size();
#endif
        if (values.rows() == 0 && labels.size() == 0)
        {
            _showXticks = HIDE_TICK;
            return;
        }

        if (values.rows() != labels.size())
        {
            qCritical() << "xticks(): the amount of values and labels must match!";
            return;
        }

        for (int i = 0; i < values.rows(); i++)
            _xTicks.push_back(QPair<QString, qreal>(labels[i], values[i]));

        _showXticks = SHOW_CUSTOM_TICK;

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "xticks(): xticks.sz=" << _xTicks.size();
        for (int i = 0; i < _xTicks.size(); i++)
            qDebug() << "\t" << _xTicks[i].second << " = " << _xTicks[i].first;
#endif
    }

    /* yticks(): sets the y-limits of the current tick locations and labels.
     */
    void yticks(const Eigen::ArrayXf& values, const QVector<QString>& labels)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "yticks(): values.sz=" << values.rows() << " labels.sz=" << labels.size();
#endif
        if (values.rows() == 0 && labels.size() == 0)
        {
            _showYticks = HIDE_TICK;
            return;
        }

        if (values.rows() != labels.size())
        {
            qCritical() << "PlotLib::xticks(): the amount of values and labels must match!" ;
            return;
        }

        for (int i = 0; i < values.rows(); i++)
            _yTicks.push_back(QPair<QString, qreal>(labels[i], values[i]));

        _showYticks = SHOW_CUSTOM_TICK;

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "yticks(): yticks.sz=" << _yTicks.size();
        for (int i = 0; i < _yTicks.size(); i++)
            qDebug() << "\t" << _yTicks[i].second << " = " << _yTicks[i].first;
#endif
    }

    /* locator_params(): reduce or increase the amount of ticks for each axis.
     */
    void locator_params(QString axis, int nbins)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "locator_params(): axis=" << axis << " nbins=" << nbins;
#endif
        if (axis == "x")
        {
            _xTickCount = nbins;
        }
        else if (axis == "y")
        {
            _yTickCount = nbins;
        }
        else if (axis == "both")
        {
            _yTickCount = _xTickCount = nbins;
        }
        else
        {
            qCritical() << "locator_params(): '" << axis << "' is not a valid option.";
            return;
        }
    }
  
    template<class T, class ... Args>
    typename std::enable_if<!is_matrix_expression<T>::value>::type plot(const Eigen::ArrayXf& y, const T& arg1, const Args&... args)
    {
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "plot(y): marker=" << marker << " alpha=" << alpha <<
            " color=" << color << " edgecolor=" << edgecolor <<
            " linewidth=" << linewidth << " markersize=" << markersize;
#endif
        if (y.rows() == 0)
        {
            qCritical() << "plot(y): axis size must be > 0 but its " << y.rows();
            exit(-1);
        }

        // number of x values needed to accompany the y values
        int num_items = y.rows();

        // make up X data and plot into series, but take into account that xlim()
        // could have been called with the start and end of x series.
        Eigen::ArrayXf x = Eigen::ArrayXf(y.rows());
        qreal x_inc = 1;
        if (_customLimits && (_xMin != _xMax))
        {
            qreal xrange = _xMax - _xMin;
            x_inc = xrange / y.rows();
        }

        qreal x_value = _xMin;
        for (int i = 0; i < num_items; i++)
        {
            x[i] = x_value;
            x_value += x_inc;

#if (DEBUG > 1) && (DEBUG < 3)
            qDebug() << "plot(y): generated x[" << i << "]=" << x[i];
#endif
        }
        plotXY(x, y, arg1, args...);
    }


    /* plot(): called when user needs to put data on a chart.
     * x: an array that stores x axis values.
     * y: an array that stores y axis values.     
     * marker: chart types. "-" for line plot and "o" for scatter plot.
     * alpha: defines the transparency level of the color.
     * color: defines the color used to draw the data.
     * edgecolor: defines the edge color of "o" marker.
     * linewidth: defines the width of the pen used to draw "-" marker.
     * markersize: defines the size of "o" marker.
     */
	template<class T, class ... Args>
	typename std::enable_if<is_matrix_expression<T>::value>::type plot(const Eigen::ArrayXf& x, const T& y, const Args&... args)
	{
		plotXY(x, y, args...);
	}

    template<class ... Args>
	void plotXY(const Eigen::ArrayXf& x, const Eigen::ArrayXf& y, const Args&... args)
    {
        const QString marker = GetKeywordInputDefault<tag::marker>(DEFAULT_MARKER, args...);
        const QString label  = GetKeywordInputDefault<tag::label>(DEFAULT_LEGEND, args...);
        const qreal& alpha = GetKeywordInputDefault<tag::alpha>(DEFAULT_ALPHA, args...);
        const QColor color = GetKeywordInputDefault<tag::color>(DEFAULT_COLOR, args...);
        const quint32& linewidth = GetKeywordInputDefault<tag::linewidth>(DEFAULT_LINEW, args...);
        const QColor edgecolor = GetKeywordInputDefault<tag::edgecolor>(DEFAULT_EDGECOLOR, args...);
        const qreal& markersize = GetKeywordInputDefault<tag::markersize>(DEFAULT_MARKERSZ, args...);

        
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "plot(x,y): marker:" << marker << " alpha:" << alpha <<
                    " color:" << color << " edgecolor:" << edgecolor <<
                    " linewidth:" << linewidth << " markersize:" << markersize;
#endif

        if (marker != "-" && marker != "--" && marker != "." && marker != "o" && marker != "s")
        {
            qCritical() << "plot(x,y): unknown marker '" << marker << "'.";
            return;
        }

        if (x.rows() != y.rows())
        {
            qCritical() << "plot(x,y): x.sz=" << x.cols() << " != y.sz=" << y.rows();
            exit(-1);
        }

        if (x.rows() == 0 || y.rows() == 0)
        {
            qCritical() << "plot(x,y): axis size must be > 0 but it is x.sz=" << x.rows() << " y.sz=" << y.rows();
            exit(-1);
        }

        // Make a copy because it's show() who setup these things
        _legend = label;

        // find min and max values to define the range of the X axis
        qreal xMin = x.minCoeff();
        qreal xMax = x.maxCoeff();
        if (xMin < _xMin)
            _xMin = xMin;
        if (xMax > _xMax)
            _xMax = xMax;

        // find min and max values to stablish the range of the Y axis
        // however, if a new series brings more xtreme values, we need to respect that!
        qreal yMin = y.minCoeff();
        qreal yMax = y.maxCoeff();
        if (yMin < _yMin)
            _yMin = yMin;
        if (yMax > _yMax)
            _yMax = yMax;

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "plot(x,y): xrange [" << _xMin << "," << _xMax <<
                     "]  yrange [" << _yMin << "," << _yMax << "]";
#endif

        std::shared_ptr<QtCharts::QXYSeries> series;
        auto itr = _seriesVec.find(_legend);
        if(itr != _seriesVec.end()){
            series = *itr;
        }else{
            if (marker == "o" || marker == "s") // it's a scatter plot!
            {
    #if (DEBUG > 1) && (DEBUG < 3)
                qDebug() << "plot(x,y): scatter plot";
    #endif
                QtCharts::QScatterSeries* s = new QtCharts::QScatterSeries();
                s->setMarkerSize(markersize); // symbol size

                if (marker == "o")
                    s->setMarkerShape(QtCharts::QScatterSeries::MarkerShapeCircle);

                if (marker == "s")
                    s->setMarkerShape(QtCharts::QScatterSeries::MarkerShapeRectangle);
                
                series.reset((QtCharts::QXYSeries*)s);
                series->setUseOpenGL(true);
            }
            else // draw line
            {
    #if (DEBUG > 1) && (DEBUG < 3)
                qDebug() << "plot(x,y): line plot";
    #endif
                series.reset(new QtCharts::QLineSeries());
                series->setUseOpenGL(true);
            }
        }
        // Call a string parser! Ex: "label=Trump Tweets" becomes "Trump Tweets"
        _parseLegend();
        if (_legend.size())
        {
#if (DEBUG > 1) && (DEBUG < 3)
            qDebug() << "plot(x,y): label=" << _legend;
#endif
            series->setName(_legend);
        }
        
        if(series->count() == x.rows()){
            for (int i = 0; i < x.rows(); i++)
                series->replace(i, x[i], y[i]);
        }else{
            series->clear();
            for (int i = 0; i < x.rows(); i++)
            {
#if (DEBUG > 1) && (DEBUG < 3)
                qDebug() << "plot(x,y): x[" << i << "]=" << x[i] << " y[" << i << "]=" << y[i];
#endif
                series->append(x[i], y[i]);
            }
        }

        // Customize series color and transparency
        QColor fillColor = color;
        if (fillColor == DEFAULT_COLOR)
            fillColor = _colors[_colorIdx++];
        fillColor.setAlphaF(alpha);

        QPen pen = series->pen();
        pen.setWidth(linewidth);

        if (marker == "o" || marker == "s")
        {
            if (edgecolor == DEFAULT_EDGECOLOR)
            {
                pen.setColor(fillColor);        // outline should be invisible
#if (DEBUG > 1) && (DEBUG < 3)
                qDebug() << "plot(x,y): fillColor=" << fillColor;
#endif
            }
            else
            {
#if (DEBUG > 1) && (DEBUG < 3)
                qDebug() << "plot(x,y): edgecolor=" << edgecolor;
#endif
                QColor edgeColor = edgecolor;
                edgeColor.setAlphaF(alpha);
                pen.setColor(edgeColor);       // create circle outline
            }
        }
        else if (marker == "--")
        {
            pen.setStyle(Qt::DashLine);
            pen.setColor(fillColor);
        }
        else if (marker == ".")
        {
            pen.setStyle(Qt::DotLine);
            pen.setColor(fillColor);
        }
        else // marker == "-"
        {
            pen.setColor(fillColor);
        }

        series->setPen(pen);
        series->setBrush(QBrush(fillColor));

        if (_colorIdx >= _colors.size())
            _colorIdx = 0;

        //_seriesVec.push_back(series);
        _seriesVec[_legend] = series;

#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "plot(x,y): -----";
#endif
    }    

    /* show(): displays all the data added through plot() calls.
     */
    void show()
    {        
#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "show(): " << _title;
#endif
        if (!_seriesVec.size())
        {
            qCritical() << "show()!!! Must set the data with plot() before show().";
            return;
        }

        /* Customize chart title */

        QFont font;
        font.setPixelSize(12);
        font.setWeight(QFont::Bold);
        _chart->setTitleFont(font);
        _chart->setTitle(_title);

        //TODO: investigate detaching the legend for custom positioning
        //https://doc.qt.io/qt-5/qtcharts-legend-example.html

        if (_legendPos.size())
        {
            if (_legendPos == "lower center")      // 9
                _chart->legend()->setAlignment(Qt::AlignBottom);
            else if (_legendPos == "upper center") // 8
                _chart->legend()->setAlignment(Qt::AlignTop);
            else if (_legendPos == "center right") // 7
                _chart->legend()->setAlignment(Qt::AlignRight);
            else if (_legendPos == "center left") // 6
                _chart->legend()->setAlignment(Qt::AlignLeft);
            else
            {
                qCritical() << "show()!!!" << _legendPos << " is not a valid legend position.";
                _chart->legend()->setAlignment(Qt::AlignBottom);
            }
        }

        if (_legend.size())
            _chart->legend()->setVisible(true);
        else
            _chart->legend()->setVisible(false);

        /* Customize X, Y axis and categories */

#if (DEBUG > 1) && (DEBUG < 3)
        qDebug() << "show(): xrange [" << _xMin << "," << _xMax << "] " <<
                     " yrange [" << _yMin << "," << _yMax << "]";
#endif
        
        QPen axisPen(Qt::black); // default axis line color and width
        axisPen.setWidth(1);

        QtCharts::QValueAxis* axisX = NULL;
        QtCharts::QCategoryAxis* categoryX = NULL;
        if (_showXticks == SHOW_TICK || _showXticks == HIDE_TICK)
        {
            bool add = true;
            if (_xAxisBottom){
                axisX = dynamic_cast<QtCharts::QValueAxis*>(_xAxisBottom);
                add = false;
            }else{
                axisX = new QtCharts::QValueAxis;
            }
            axisX->setGridLineVisible(_enableGrid);
            axisX->setTitleText(_xLabel);
            axisX->setLinePen(axisPen);
            axisX->setRange(_xMin, _xMax);
            axisX->setTickCount(_xTickCount);
            if (!_customLimits) axisX->applyNiceNumbers();
            if(add)
                _chart->addAxis(axisX, Qt::AlignBottom);
            _xAxisBottom = axisX;
            if (_showXticks == HIDE_TICK)
                axisX->setLabelsVisible(false);
        }
        else if (_showXticks == SHOW_CUSTOM_TICK)
        {
            categoryX = new QtCharts::QCategoryAxis();
            categoryX->setGridLineVisible(_enableGrid);
            categoryX->setLinePen(axisPen);

            if (_showXticks && _xTicks.size())
                for (int i = 0; i < _xTicks.size(); i++)
                {
#if (DEBUG > 1) && (DEBUG < 3)
                    qDebug() << "show(): xtick[" << i << "]=(" << _xTicks[i].second << " , " <<
                                _xTicks[i].first << ")";
#endif
                    categoryX->append(_xTicks[i].first, _xTicks[i].second);
                }

            categoryX->setRange(_xMin, _xMax);
            categoryX->setTickCount(_xTicks.size());
            if(_xAxisBottom)
                _chart->removeAxis(_xAxisBottom);
            _chart->addAxis(categoryX, Qt::AlignBottom);
            _xAxisBottom = categoryX;
        }

        QtCharts::QValueAxis* axisY = NULL;
        QtCharts::QCategoryAxis* categoryY = NULL;
        if (_showYticks == SHOW_TICK || _showYticks == HIDE_TICK) // this is the default ticks setup
        {
            bool add = true;
            if(_yAxisLeft){
                axisY = dynamic_cast<QtCharts::QValueAxis*>(_yAxisLeft);
                add = false;
            }else{
                axisY = new QtCharts::QValueAxis;
            }
            
            axisY->setGridLineVisible(_enableGrid);
            axisY->setTitleText(_yLabel);
            axisY->setLinePen(axisPen);
            axisY->setRange(_yMin, _yMax);
            axisY->setTickCount(_yTickCount);
            if (!_customLimits) axisY->applyNiceNumbers();
            if(add)
                _chart->addAxis(axisY, Qt::AlignLeft);
            _yAxisLeft = axisY;
            
            if (_showYticks == HIDE_TICK)
                axisY->setLabelsVisible(false);
        }
        else if (_showYticks == SHOW_CUSTOM_TICK) // this is for user defined ticks
        {
            categoryY = new QtCharts::QCategoryAxis();
            categoryY->setGridLineVisible(_enableGrid);
            categoryY->setLinePen(axisPen);

            if (_showYticks && _yTicks.size() > 0)
                for (int i = 0; i < _yTicks.size(); i++)
                {
#if (DEBUG > 1) && (DEBUG < 3)
                    qDebug() << "show(): ytick[" << i << "]=(" << _yTicks[i].second << " , " <<
                                _yTicks[i].first << ")";
#endif
                    categoryY->append(_yTicks[i].first, _yTicks[i].second);
                }

            categoryY->setRange(_yMin, _yMax);
            categoryY->setTickCount(_yTicks.size());
            if(_yAxisLeft)
                _chart->removeAxis(_yAxisLeft);
            _chart->addAxis(categoryY, Qt::AlignLeft);
            _yAxisLeft = categoryY;
        }

        /* Other possible customizations such as margins and background color */
        // Remove (fat) exterior margins from QChart
        _chart->layout()->setContentsMargins(0, 0, 0, 0);
        _chart->setBackgroundRoundness(0);

        /* Add series of data */
        _chart->removeAllSeries();
        for(auto itr : _seriesVec)
        {
            _chart->addSeries(itr.get());

            if (_showXticks == SHOW_TICK || _showXticks == HIDE_TICK)
            {
                itr->attachAxis(axisX);
            }
            else if (_showXticks == SHOW_CUSTOM_TICK)
            {
                itr->attachAxis(categoryX);
            }

            if (_showYticks == SHOW_TICK || _showYticks == HIDE_TICK)
            {
                itr->attachAxis(axisY);
            }
            else if (_showYticks == SHOW_CUSTOM_TICK)
            {
                itr->attachAxis(categoryY);
            }
        }

        _chartView->setRenderHint(QPainter::Antialiasing);
        _chartView->resize(600, 400);

        // Take a screenshot of the widget before it's destroyed
        // so it can be saved later, when savefig() is invoked after show().
        

        _chartView->show();

        // This loop blocks execution & waits for the window to be destroyed.
        // However, is this chart is supposed to be a real widget, then do none of this.
        if (!_isWidget)
        {
            _pixmap = _chartView->grab();
            _chartView->setAttribute(Qt::WA_DeleteOnClose); // This deletes _chartView!
            QEventLoop loop;
            QObject::connect(_chartView, SIGNAL(destroyed()), &loop, SLOT(quit()));
            loop.exec();

            // _chartView was automatically deleted. Release all the other resources!
            //_seriesVec.clear();
        }

#if (DEBUG > 0) && (DEBUG < 2)
        qDebug() << "show(): -----" ;
#endif
    }

    void clear(){
        _seriesVec.clear();
    }


private:

    bool _is_marker(const QString& cmd)
    {
        if (cmd == "-" || cmd == "--" || cmd == "." || cmd == "o" || cmd == "s")
            return true;

        return false; // cmd is a label for the legend
    }

    void _check_cmds_are_good(const QString& cmd1, const QString& cmd2)
    {
        if (_is_marker(cmd1) && !_is_marker(cmd2))
            return;

        if (!_is_marker(cmd1) && _is_marker(cmd2))
            return;

        qCritical() << "_check_cmds_are_good()!!! Only one marker and one label are allowed.";
        exit(-1);
    }

    void _parseLegend()
    {
        if (!_legend.size())
            return;

        QRegExp equalsRegex("(\\=)"); //RegEx for '='
        QStringList wordList = _legend.split(equalsRegex);

        // step1: trim spaces
        for (int i = 0; i < wordList.size(); i++)
            wordList[i] = wordList[i].trimmed();

        // step 2: delete empty strings
        QMutableStringListIterator it(wordList); // pass list as argument
        while (it.hasNext())
        {
            if (!it.next().size())
                it.remove();
        }

        int i = -1;
        QStringList::iterator iter = std::find(wordList.begin(), wordList.end(), "label");
        if (iter != wordList.end())
            i = iter - wordList.begin(); // if "loc" is in the list, i has the index

        // ok, "label" exists && there's another word after it on the list
        if (i >= 0 && i+1 < wordList.size())
            _legend = wordList[i+1];
        else
            _legend.clear();
    }

    QString _parseLegendPos(QString cmd)
    {
        if (!cmd.size())
            return QString();

        QRegExp equalsRegex("(\\=)"); //RegEx for '='
        QStringList wordList = cmd.split(equalsRegex);

        // step1: trim spaces
        for (int i = 0; i < wordList.size(); i++)
            wordList[i] = wordList[i].trimmed();

        // step 2: delete empty strings
        QMutableStringListIterator it(wordList); // pass list as argument
        while (it.hasNext())
        {
            if (!it.next().size())
                it.remove();
        }

        int i = -1;
        QStringList::iterator iter = std::find(wordList.begin(), wordList.end(), "loc");
        if (iter != wordList.end())
            i = iter - wordList.begin(); // if "loc" is in the list, i has the index

        // ok, "label" exists && there's another word after it on the list
        if (i >= 0 && i+1 < wordList.size())
            return wordList[i+1];

        return QString();
    }

    QPixmap _pixmap;                                // show() screenshots the widget, savefig() writes it on the disk
    QtCharts::QChart* _chart;                       // manages the graphical representation of the chart's series, legends & axes
    QtCharts::QChartView* _chartView;               // standalone widget that can display charts
    QMap<QString, std::shared_ptr<QtCharts::QXYSeries>> _seriesVec;      // every plot() creates a new series of data that is stored here
    

    bool _isWidget;                                // true: show() doesn't block so this can be used as widget
    QString _legend;
    QString _legendPos;

    QVector<QPair<QString, qreal> > _xTicks;       // user defined <label, endValue> ticks that replace default ticks
    QVector<QPair<QString, qreal> >  _yTicks;
    int _showXticks;                               // flag that show/hides the exhibition of ticks on the X axis
    int _showYticks;                               // flag that show/hides the exhibition of ticks on the Y axis
    int _xTickCount;                              // number of ticks displayed on the X axis
    int _yTickCount;                              // number of ticks displayed on the Y axis

    QString _title;                                 // Chart title
    QString _yLabel;                                // String that appears to the left of the Y axis
    QString _xLabel;                                // String that appears below the X axis

    QVector<QColor> _colors;                        // Vector that stores chart's predefined colours
    int _colorIdx;                                 // Every plot() increases this index so the next has a different colour

    bool _customLimits;                            // If the user has informed new limits through xlim(), ylim(), or axis()
    qreal _xMin;                                   // X axis min limit
    qreal _xMax;                                   // X axis max limit
    qreal _yMin;                                   // Y axis min limit
    qreal _yMax;                                   // Y axis max limit

    bool _enableGrid;                              // flag that show/hides the background grid
    QtCharts::QAbstractAxis* _yAxisLeft;
    QtCharts::QAbstractAxis* _yAxisRight;
    QtCharts::QAbstractAxis* _xAxisBottom;
    QtCharts::QAbstractAxis* _xAxisTop;
};
