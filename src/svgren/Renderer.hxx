#pragma once

#include <vector>

#include <utki/config.hpp>

#if M_OS == M_OS_WINDOWS || M_OS_NAME == M_OS_NAME_IOS
#	include <cairo.h>
#else
#	include <cairo/cairo.h>
#endif

#include <svgdom/visitor.hpp>
#include <svgdom/finder.hpp>
#include <svgdom/style_stack.hpp>
#include <svgdom/elements/aspect_ratioed.hpp>

#include "config.hpp"
#include "Surface.hxx"
#include "util.hxx"

namespace svgren{

class Renderer : public svgdom::const_visitor{
public:
	cairo_t* cr;
	
	svgdom::finder finder;
	
	const real dpi;
	
	bool isOutermostElement = true;
	
	std::array<real, 2> viewport; // width, height
	
	// this bounding box is used for gradients
	std::array<real, 2> userSpaceShapeBoundingBoxPos = {{0, 0}};
	std::array<real, 2> userSpaceShapeBoundingBoxDim = {{0, 0}};
	
	// this bounding box is used for filter region calculation.
	DeviceSpaceBoundingBox deviceSpaceBoundingBox;
	
	svgdom::style_stack styleStack;
	
	Surface background; // for accessing background image from filter effects
	
	// blit surface to current cairo surface
	void blit(const Surface& s);
	
	real lengthToPx(const svgdom::Length& l, unsigned coordIndex = 0)const noexcept;
	
	void applyCairoTransformation(const svgdom::Transformable::Transformation& t);
	
	void applyTransformations(const decltype(svgdom::Transformable::transformations)& transformations);
	
	void setCairoPatternSource(cairo_pattern_t& pat, const svgdom::Gradient& g, const svgdom::style_stack& ss);
	
	void setGradient(const std::string& id);
	
	void applyFilter(const std::string& id);
	void applyFilter();
	
	void updateCurBoundingBox();
	
	void renderCurrentShape(bool isCairoGroupPushed);
	
	void applyViewBox(const svgdom::ViewBoxed& e, const svgdom::AspectRatioed& ar);
	
	void renderSvgElement(
			const svgdom::Container& c,
			const svgdom::Styleable& s,
			const svgdom::ViewBoxed& v,
			const svgdom::AspectRatioed& a,
			const svgdom::Length& x,
			const svgdom::Length& y,
			const svgdom::Length& width,
			const svgdom::Length& height
		);
	
	const decltype(svgdom::Transformable::transformations)& gradientGetTransformations(const svgdom::Gradient& g);
	svgdom::CoordinateUnits_e gradientGetUnits(const svgdom::Gradient& g);
	
	svgdom::Length gradientGetX1(const svgdom::LinearGradientElement& g);
	svgdom::Length gradientGetY1(const svgdom::LinearGradientElement& g);
	svgdom::Length gradientGetX2(const svgdom::LinearGradientElement& g);
	svgdom::Length gradientGetY2(const svgdom::LinearGradientElement& g);
	
	svgdom::Length gradientGetCx(const svgdom::RadialGradientElement& g);
	svgdom::Length gradientGetCy(const svgdom::RadialGradientElement& g);
	svgdom::Length gradientGetR(const svgdom::RadialGradientElement& g);
	svgdom::Length gradientGetFx(const svgdom::RadialGradientElement& g);
	svgdom::Length gradientGetFy(const svgdom::RadialGradientElement& g);
	
	const decltype(svgdom::Container::children)& gradientGetStops(const svgdom::gradient& g);
	const decltype(svgdom::styleable::styles)& gradient_get_styles(const svgdom::gradient& g);
	const decltype(svgdom::styleable::classes)& gradient_get_classes(const svgdom::gradient& g);
	svgdom::gradient::spread_method gradientGetSpreadMethod(const svgdom::gradient& g);
	decltype(svgdom::styleable::presentation_attributes) gradient_get_presentation_attributes(const svgdom::gradient& g);
	
	bool isInvisible();
	bool isGroupInvisible();
	
public:
	Renderer(
			cairo_t* cr,
			real dpi,
			std::array<real, 2> canvasSize,
			const svgdom::SvgElement& root
		);

	// WORKAROUND: MSVS compiler complains about cannot access protected member,
	//             Declare public method which calls protected one.
	void relayAccept(const svgdom::Container& e){
		this->const_visitor::relay_accept(e);
	}

	void visit(const svgdom::GElement& e)override;
	void visit(const svgdom::UseElement& e)override;
	void visit(const svgdom::SvgElement& e)override;
	void visit(const svgdom::PathElement& e)override;
	void visit(const svgdom::CircleElement& e) override;
	void visit(const svgdom::PolylineElement& e) override;
	void visit(const svgdom::PolygonElement& e) override;
	void visit(const svgdom::LineElement& e) override;
	void visit(const svgdom::EllipseElement& e) override;
	void visit(const svgdom::RectElement& e) override;

	void default_visit(const svgdom::Element& e, const svgdom::Container& c) override{
		// do nothing by default
	}
};

}
