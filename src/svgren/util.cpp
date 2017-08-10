#include "util.hxx"

#include <cstring>
#include <cmath>
#include <vector>

#include <utki/debug.hpp>
#include <utki/math.hpp>

using namespace svgren;

namespace{
void boxBlurHorizontal(
		std::uint8_t* dst,
		const std::uint8_t* src,
		unsigned stride,
		unsigned numRows,
		unsigned boxSize,
		unsigned boxOffset,
		unsigned channel
	)
{
	if(boxSize == 0){
		return;
	}
	for(unsigned y = 0; y != numRows; ++y){
		unsigned sum = 0;
		for(unsigned i = 0; i != boxSize; ++i){
			int pos = i - boxOffset;
			pos = std::max(pos, 0);
			pos = std::min(pos, int(stride - 1));
			sum += src[(stride * y + pos) * sizeof(std::uint32_t) + channel];
		}
		for(unsigned x = 0; x != stride; ++x){
			int tmp = x - boxOffset;
			int last = std::max(tmp, 0);
			int next = std::min(tmp + boxSize, stride - 1);

			dst[(stride * y + x) * sizeof(std::uint32_t) + channel] = sum / boxSize;

			sum += src[(stride * y + next) * sizeof(std::uint32_t) + channel]
					- src[(stride * y + last) * sizeof(std::uint32_t) + channel];
		}
	}
}
}

namespace{
void boxBlurVertical(
		std::uint8_t* dst,
		const std::uint8_t* src,
		unsigned stride,
		unsigned numRows,
		unsigned boxSize,
		unsigned boxOffset,
		unsigned channel
	)
{
	if(boxSize == 0){
		return;
	}
	for(unsigned x = 0; x != stride; ++x){
		unsigned sum = 0;
		for(unsigned i = 0; i != boxSize; ++i){
			int pos = i - boxOffset;
			pos = std::max(pos, 0);
			pos = std::min(pos, int(numRows - 1));
			sum += src[(stride * pos + x) * sizeof(std::uint32_t) + channel];
		}
		for(unsigned y = 0; y != numRows; ++y){
			int tmp = y - boxOffset;
			int last = std::max(tmp, 0);
			int next = std::min(tmp + boxSize, numRows - 1);

			dst[(stride * y + x) * sizeof(std::uint32_t) + channel] = sum / boxSize;

			sum += src[(x + stride * next) * sizeof(std::uint32_t) + channel]
					- src[(x + stride * last) * sizeof(std::uint32_t) + channel];
		}
	}
}
}



void svgren::cairoImageSurfaceBlur(cairo_surface_t* surface, std::array<real, 2> stdDeviation){
	if(cairo_image_surface_get_format (surface) != CAIRO_FORMAT_ARGB32){
		TRACE(<< "cairo_image_surface_blur(): ERROR: wrong surface format, only ARGB32 is supported." << std::endl)
		return;
	}

	int width = cairo_image_surface_get_width(surface);
	int height = cairo_image_surface_get_height(surface);
	
	//NOTE: see https://www.w3.org/TR/SVG/filters.html#feGaussianBlurElement for Gaussian Blur approximation algorithm.
	
	std::array<unsigned, 2> d;
	for(unsigned i = 0; i != 2; ++i){
		d[i] = unsigned(float(stdDeviation[i]) * 3 * std::sqrt(2 * utki::pi<float>()) / 4 + 0.5f);
	}
	
	std::uint8_t* src = cairo_image_surface_get_data(surface);
	
	std::vector<std::uint8_t> tmp(width * height * sizeof(std::uint32_t));
	
	std::array<unsigned, 3> hBoxSize;
	std::array<unsigned, 3> hOffset;
	std::array<unsigned, 3> vBoxSize;
	std::array<unsigned, 3> vOffset;
	if(d[0] % 2 == 0){
		hOffset[0] = d[0] / 2;
		hBoxSize[0] = d[0];
		hOffset[1] = d[0] / 2 + 1;
		hBoxSize[1] = d[0];
		hOffset[2] = d[0] / 2;
		hBoxSize[2] = d[0] + 1;
	}else{
		hOffset[0] = d[0] / 2;
		hBoxSize[0] = d[0];
		hOffset[1] = d[0] / 2;
		hBoxSize[1] = d[0];
		hOffset[2] = d[0] / 2;
		hBoxSize[2] = d[0];
	}
	
	if(d[1] % 2 == 0){
		vOffset[0] = d[0] / 2;
		vBoxSize[0] = d[0];
		vOffset[1] = d[0] / 2 + 1;
		vBoxSize[1] = d[0];
		vOffset[2] = d[0] / 2;
		vBoxSize[2] = d[0] + 1;
	}else{
		vOffset[0] = d[0] / 2;
		vBoxSize[0] = d[0];
		vOffset[1] = d[0] / 2;
		vBoxSize[1] = d[0];
		vOffset[2] = d[0] / 2;
		vBoxSize[2] = d[0];
	}
	
	for(auto channel = 0; channel != 4; ++channel){
		boxBlurHorizontal(&*tmp.begin(), src, width, height, hBoxSize[0], hOffset[0], channel);
	}
	for(auto channel = 0; channel != 4; ++channel){
		boxBlurHorizontal(src, &*tmp.begin(), width, height, hBoxSize[1], hOffset[1], channel);
	}
	for(auto channel = 0; channel != 4; ++channel){
		boxBlurHorizontal(&*tmp.begin(), src, width, height, hBoxSize[2], hOffset[2], channel);
	}
	for(auto channel = 0; channel != 4; ++channel){
		boxBlurVertical(src, &*tmp.begin(), width, height, vBoxSize[0], vOffset[0], channel);
	}
	for(auto channel = 0; channel != 4; ++channel){
		boxBlurVertical(&*tmp.begin(), src, width, height, vBoxSize[1], vOffset[1], channel);
	}
	for(auto channel = 0; channel != 4; ++channel){
		boxBlurVertical(src, &*tmp.begin(), width, height, vBoxSize[2], vOffset[2], channel);
	}
}



void svgren::cairoRelQuadraticCurveTo(cairo_t *cr, double x1, double y1, double x, double y){
	cairo_rel_curve_to(cr,
			2.0 / 3.0 * x1,
			2.0 / 3.0 * y1,
			2.0 / 3.0 * x1 + 1.0 / 3.0 * x,
			2.0 / 3.0 * y1 + 1.0 / 3.0 * y,
			x,
			y
		);
}

void svgren::cairoQuadraticCurveTo(cairo_t *cr, double x1, double y1, double x, double y){
	double x0, y0; //current point, absolute coordinates
	if (cairo_has_current_point(cr)) {
		cairo_get_current_point(cr, &x0, &y0);
	}
	else {
		cairo_move_to(cr, 0, 0);
		x0 = 0;
		y0 = 0;
	}
	cairo_curve_to(cr,
			2.0 / 3.0 * x1 + 1.0 / 3.0 * x0,
			2.0 / 3.0 * y1 + 1.0 / 3.0 * y0,
			2.0 / 3.0 * x1 + 1.0 / 3.0 * x,
			2.0 / 3.0 * y1 + 1.0 / 3.0 * y,
			x,
			y
		);
}

real svgren::degToRad(real deg){
	return deg * utki::pi<real>() / real(180);
}

std::array<real, 2> svgren::rotate(real x, real y, real angle){
    return {{x * std::cos(angle) - y * std::sin(angle), y * std::cos(angle) + x * std::sin(angle)}};
}

real svgren::pointAngle(real cx, real cy, real px, real py){
    return std::atan2(py - cy, px - cx);
}

CairoContextSaveRestore::CairoContextSaveRestore(cairo_t* cr) :
		cr(cr)
{
	ASSERT(this->cr)
	cairo_save(this->cr);
}

CairoMatrixSaveRestore::CairoMatrixSaveRestore(cairo_t* cr) :
		cr(cr)
{
	ASSERT(this->cr)
	cairo_get_matrix(this->cr, &this->m);
}

CairoContextSaveRestore::~CairoContextSaveRestore()noexcept{
	cairo_restore(this->cr);
}

CairoMatrixSaveRestore::~CairoMatrixSaveRestore()noexcept{
	cairo_set_matrix(this->cr, &this->m);
}