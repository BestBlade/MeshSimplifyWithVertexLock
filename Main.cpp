// Simple wrapper for Sven Forstmann's mesh simplification tool
//
// Loads a OBJ format mesh, decimates mesh, saves decimated mesh as OBJ format
// http://voxels.blogspot.com/2014/05/quadric-mesh-simplification-with-source.html
// https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
//To compile for Linux/OSX (GCC/LLVM)
//  g++ Main.cpp -O3 -o simplify
//To compile for Windows (Visual Studio)
// vcvarsall amd64
// cl /EHsc Main.cpp /osimplify
//To execute
//  ./simplify wall.obj out.obj 0.04
//
// Pascal Version by Chris Roden:
// https://github.com/neurolabusc/Fast-Quadric-Mesh-Simplification-Pascal-
//
//#define RUN_IN_CMD
#include "Simplify.h"
#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <time.h>  // clock_t, clock, CLOCKS_PER_SEC

void showHelp(const char* argv[]) {
    const char* cstr = (argv[0]);
    printf("Usage: %s <input> <output> <ratio> <agressiveness)\n", cstr);
    printf(" Input: name of existing OBJ format mesh\n");
    printf(" Output: name for decimated OBJ format mesh\n");
    printf(" Ratio: (default = 0.5) for example 0.2 will decimate 80%% of triangles\n");
    printf(" Agressiveness: (default = 7.0) faster or better decimation\n");
    printf(" Lock Group: (default = root) name of group OBJ");
    printf("Examples :\n");
#if defined(_WIN64) || defined(_WIN32)
    printf("  %s c:\\dir\\in.obj c:\\dir\\out.obj 0.2\n", cstr);
#else
    printf("  %s ~/dir/in.obj ~/dir/out.obj 0.2\n", cstr);
#endif
} //showHelp()

#if RUN_IN_CMD
int simplify(int argc, const char* argv[])
#else
int simplify(int argc, std::vector<std::string> argv)
#endif // RUN_IN_CMD
{
    printf("Mesh Simplification (C)2014 by Sven Forstmann in 2014, MIT License (%zu-bit)\n", sizeof(size_t) * 8);
    if (argc < 3) {
        //showHelp(argv);
        return EXIT_SUCCESS;
    }
#ifdef RUN_IN_CMD
    Simplify::load_obj(argv[1]);
#else
    Simplify::load_obj(argv[1].c_str());
#endif
    if ((Simplify::triangles.size() < 3) || (Simplify::vertices.size() < 3))
        return EXIT_FAILURE;
    //Simplify::lock_group("edge");
    int target_count = Simplify::triangles.size() >> 1;
    if (argc > 3) {
#ifdef RUN_IN_CMD
        float reduceFraction = atof(argv[3]);
#else
        float reduceFraction = atof(argv[3].c_str());
#endif // DEBUG_IN_VS
        if (reduceFraction > 1.0) reduceFraction = 1.0; //lossless only
        if (reduceFraction <= 0.0) {
            printf("Ratio must be BETWEEN zero and one.\n");
            return EXIT_FAILURE;
        }
#ifdef RUN_IN_CMD
        target_count = round((float)Simplify::triangles.size() * atof(argv[3]));
#else
        target_count = round((float)Simplify::triangles.size() * atof(argv[3].c_str()));
#endif // DEBUG_IN_VS
    }
    if (target_count < 4) {
        printf("Object will not survive such extreme decimation\n");
        return EXIT_FAILURE;
    }
    double agressiveness = 7.0;
    if (argc > 4) {
#ifdef RUN_IN_CMD
        agressiveness = atof(argv[4]);
#else
        agressiveness = atof(argv[4].c_str());
#endif // DEBUG_IN_VS
    }
    if (argc > 5) {
        for (int i = 5; i < argc; ++i)
        {
#ifdef RUN_IN_CMD
            Simplify::lock_group(argv[i]);
#else
            Simplify::lock_group(argv[i].c_str());
#endif // DEBUG_IN_VS
        }
    }
    clock_t start = clock();
    printf("Input: %zu vertices, %zu triangles (target %d)\n", Simplify::vertices.size(), Simplify::triangles.size(), target_count);
    int startSize = Simplify::triangles.size();
    Simplify::simplify_mesh(target_count, agressiveness, true);
    //Simplify::simplify_mesh_lossless( false);
    if (Simplify::triangles.size() >= startSize) {
        printf("Unable to reduce mesh.\n");
        return EXIT_FAILURE;
    }
#ifdef RUN_IN_CMD
    Simplify::write_obj(argv[2]);
#else
    Simplify::write_obj(argv[2].c_str());
#endif // RUN_IN_CMD
    printf("Output: %zu vertices, %zu triangles (%f reduction; %.4f sec)\n", Simplify::vertices.size(), Simplify::triangles.size()
        , (float)Simplify::triangles.size() / (float)startSize, ((float)(clock() - start)) / CLOCKS_PER_SEC);
    return EXIT_SUCCESS;
}

#ifdef RUN_IN_CMD
int main(int argc, const char* argv[]) {
    simplify(argc, argv);
#else
int main() {
    std::cout << "Work Path " << std::filesystem::current_path() << "\n";

    for (int i = 0; i < 3; ++i)
    {
        int quad_node_cnt = 1 << i;
        quad_node_cnt *= quad_node_cnt;
        for (int j = 0; j < quad_node_cnt; ++j)
        {
            std::string file_path = "./ExportMesh/Lod" + std::to_string(i);
            std::string src_file_name = "/QuadNode" + std::to_string(j) + ".obj";
            std::string dst_file_name = "/SimplifiedQuadNode" + std::to_string(j) + ".obj";

            std::vector<std::string> argv = { "", file_path + src_file_name, file_path + dst_file_name, "0.25", "7", "edge" };
            int argc = argv.size();
            simplify(argc, argv);
        }
    }
    return EXIT_SUCCESS;
#endif
}
