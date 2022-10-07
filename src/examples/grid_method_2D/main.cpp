
#include <jet/box2.h>
#include <jet/level_set_liquid_solver2.h>
#include <jet/grid_fractional_single_phase_pressure_solver2.h>
#include <jet/implicit_surface_set2.h>
#include <jet/level_set_utils.h>
#include <jet/plane2.h>
#include <jet/rigid_body_collider2.h>
#include <jet/sphere2.h>
#include <jet/surface_to_implicit2.h>
#include <jet/volume_grid_emitter2.h>

#include <jet/array_accessor1.h>
#include <jet/array_accessor2.h>
#include <jet/array_accessor3.h>
#include <jet/triangle_mesh3.h>

#include <cnpy/cnpy.h>
#include <pystring/pystring.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>
#ifdef JET_WINDOWS
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define JET_TESTS_OUTPUT_DIR "test_datasets"
#include <array3.h>
#include <logging.h>
#include <vector>
#define APP_NAME "grid_method_2D"
using namespace jet;

std::string _currentTestDir;
std::string _flag;


inline void createDirectory(const std::string& dirname) {
    std::vector<std::string> tokens;
    pystring::split(dirname, tokens, "/");
    std::string partialDir;
    for (const auto& token : tokens) {
        partialDir = pystring::os::path::join(partialDir, token);
#ifdef JET_WINDOWS
        _mkdir(partialDir.c_str());
#else
        mkdir(partialDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
    }
}

std::string getFullFilePath(const std::string& name) {
    std::string temp;
    if(_currentTestDir.empty()) return name;
    if(_flag == "") return pystring::os::path::join(_currentTestDir, name);
    else if(_flag == "vel"){
        temp = pystring::os::path::join(_currentTestDir,"vel");
        return pystring::os::path::join(temp, name);
    }else if(_flag == "phi"){
        temp = pystring::os::path::join(_currentTestDir,"phi");
        return pystring::os::path::join(temp, name);
    }else{
        // TODO other physical parameters
        return name;
    }

}


template <typename T>
void saveData(const ConstArrayAccessor1<T>& data,
              const std::string& name) {
    std::string filename = getFullFilePath(name);
    unsigned int dim[1] = {static_cast<unsigned int>(data.size())};
    cnpy::npy_save(filename, data.data(), dim, 1, "w");
}

template <typename T>
void saveData(const ConstArrayAccessor2<T>& data,
              const std::string& name) {
    std::string filename = getFullFilePath(name);
    unsigned int dim[2] = {static_cast<unsigned int>(data.height()),
                           static_cast<unsigned int>(data.width())};
    cnpy::npy_save(filename, data.data(), dim, 2, "w");
}

template <typename T>
void saveData(const ConstArrayAccessor3<T>& data,
              const std::string& name) {
    std::string filename = getFullFilePath(name);
    unsigned int dim[3] = {static_cast<unsigned int>(data.depth()),
                           static_cast<unsigned int>(data.height()),
                           static_cast<unsigned int>(data.width())};
    cnpy::npy_save(filename, data.data(), dim, 3, "w");
}


void grid2_DamBreak(double w, double h) {
    // w : emitter width [0,0.5]
    // h : emitter height [0,1]
    LevelSetLiquidSolver2 solver;

    auto data = solver.gridSystemData();
    double dx = 1.0 / 128.0;
    data->resize(Size2(128, 128), Vector2D(dx, dx), Vector2D());

    // Source setting
    //    BoundingBox2D domain = data->boundingBox();
    ImplicitSurfaceSet2 surfaceSet;
    surfaceSet.addExplicitSurface(
        std::make_shared<Box2>(Vector2D(0.0, 0.0), Vector2D(w, h)));

    surfaceSet.addExplicitSurface(
        std::make_shared<Box2>(Vector2D(1 - w, 0), Vector2D(1, h)));

    auto sdf = solver.signedDistanceField();
    sdf->fill([&](const Vector2D& x) {
        return surfaceSet.signedDistance(x);
    });

    Array2<double> output(128, 128);
    Array2<double> p_output(128,128);
    Array3<double>  vel_output(128,128,2);
    output.forEachIndex([&](size_t i, size_t j) {
        output(i, j) = 1.0 - smearedHeavisideSdf((*sdf)(i, j) / dx);
    });

    auto velField = data->velocity();
    vel_output.forEachIndex([&](size_t i, size_t j, size_t k) {
        vel_output(i, j, 0) = velField->u(i, j);
        vel_output(i, j, 1) = velField->v(i, j);
    });

    /********** vel *******/
    _flag = "vel";
    char vel_filename[256];
    snprintf(vel_filename, sizeof(vel_filename), "vel_data.#grid2,0000.npy");
    saveData(vel_output.constAccessor(), vel_filename);
    /********* phi ********/
    _flag = "phi";
    char filename[256];
    snprintf(filename, sizeof(filename), "phi_data.#grid2,0000.npy");
    saveData(output.constAccessor(), filename);

    for (Frame frame(0, 1.0 / 60.0); frame.index < 200; frame.advance()) {
        solver.update(frame);
//        std::cout << "frame_" << frame.index << std::endl;
        output.forEachIndex([&](size_t i, size_t j) {
            output(i, j) = 1.0 - smearedHeavisideSdf((*sdf)(i, j) / dx);
        });

        /********** phi *******/
        _flag = "phi";
        snprintf(
            filename,
            sizeof(filename),
            "phi_data.#grid2,%04d.npy",
            frame.index);
        saveData(output.constAccessor(), filename);

        /********** vel *******/
        _flag = "vel";
        velField = data->velocity();
        vel_output.forEachIndex([&](size_t i, size_t j, size_t k) {
            vel_output(i, j, 0) = velField->u(i, j);
            vel_output(i, j, 1) = velField->v(i, j);
        });
        snprintf(vel_filename, sizeof(vel_filename), "vel_data.#grid2,%04d.npy",frame.index);
        saveData(vel_output.constAccessor(), vel_filename);
    }
    _flag = "";
}

int main(){
    std::ofstream logFile("test_datasets.log");
    if (logFile) {
        Logging::setAllStream(&logFile);
    }

    for(int i = 0; i < 5; i++){
        for(int j = 0;j < 5;j++){
            double w = 0.1 + i * 0.05;
            double h = 0.5 + j * 0.05;
            char filename[256];
            snprintf(filename, sizeof(filename), "case_no_obstacle_%.2lfx%.2lf",
                     w,h);

            // create sub-direction
            _currentTestDir = pystring::os::path::join(JET_TESTS_OUTPUT_DIR,std::string(filename));
            createDirectory(_currentTestDir);
            std::string phiDataDir = pystring::os::path::join(_currentTestDir,"phi");
            std::string velDataDir = pystring::os::path::join(_currentTestDir,"vel");
            createDirectory(phiDataDir);
            createDirectory(velDataDir);

//          // start simulation
            std::cout << "start case:" << _currentTestDir << std::endl;
            grid2_DamBreak(w,h);
        }
    }

    return 0;
}