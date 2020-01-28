cmake_minimum_required(VERSION 3.5)
project(Waveguide)

set(GR_REQUIRED_COMPONENTS RUNTIME BLOCKS FILTER ANALOG)
find_package(Gnuradio)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(Freetype REQUIRED)

set(INSIGHT_DIR /home/sysop/ClionProjects/Insight)
set(INSIGHT_INCLUDE_DIR ${INSIGHT_DIR})
set(INSIGHT_LIBRARIES ${INSIGHT_DIR}/build1/libInsight.a)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(. ${INSIGHT_INCLUDE_DIR} ${SDL2_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIR} /usr/include/freetype2)

set(SOURCE_FILES main.cpp sdr/SpectrumSamples.cpp sdr/SpectrumSamples.h sdr/SpectrumSampler.cpp sdr/SpectrumSampler.h sdr/FrequencyBin.cpp sdr/FrequencyBin.cpp sdr/FrequencyBin.h sdr/SampleThread.cpp sdr/SampleThread.h scenario/linear/LinearSpectrum.cpp scenario/linear/LinearSpectrum.h scenario/SimpleSpectrumRange.cpp scenario/SimpleSpectrumRange.h scenario/grid/GridSpectrum.cpp scenario/grid/GridSpectrum.h scenario/sphere/SphereSpectrum.cpp scenario/sphere/SphereSpectrum.h scenario/RotatedSpectrumRange.cpp scenario/RotatedSpectrumRange.h scenario/circular/CircularSpectrum.cpp scenario/circular/CircularSpectrum.h scenario/SimpleSpectrum.cpp scenario/SimpleSpectrum.h scenario/linear/LinearTimeSpectrum.cpp scenario/linear/LinearTimeSpectrum.h scenario/cylindrical/CylindricalSpectrum.cpp scenario/cylindrical/CylindricalSpectrum.h sdr/VectorSinkBlock.cpp sdr/VectorSinkBlock.h scenario/help/Help.cpp scenario/help/Help.h)
add_executable(Waveguide ${SOURCE_FILES})

target_link_libraries(Waveguide ${INSIGHT_LIBRARIES} ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} gnuradio-pmt gnuradio-runtime gnuradio-blocks gnuradio-analog gnuradio-fft gnuradio-filter boost_system pthread gnuradio-osmosdr)
