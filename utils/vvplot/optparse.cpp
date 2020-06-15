#include "./optparse.hpp"
#include "elementary.h"

#define H5_USE_18_API
#include <hdf5.h>

#include <string> /* std::string */
#include <cerrno> /* errno */
#include <cstdio> /* stderr */
#include <cstdint> /* uintXX_t */
#include <cstdlib> /* exit */
#include <cstring> /* strerror */
#include <sstream>
#include <cmath> /* isfinite */
#include <unistd.h> /* exec */
#include <sys/stat.h>

#include <algorithm>

#include <getopt.h>

static double &xmin = opt::rect.xmin;
static double &xmax = opt::rect.xmax;
static double &ymin = opt::rect.ymin;
static double &ymax = opt::rect.ymax;

void opt::parse(int argc, char **argv) {
	while (1) {
        static struct option long_options[] =
        {
            {"version", no_argument, 0, 'v'},
            {"help",    no_argument, 0, 'h'},

            {"png",          no_argument, &opt::ofmt, ffmt::png},
            {"tar",          no_argument, &opt::ofmt, ffmt::tar},

            {"no-gray",      no_argument, &opt::gray, 0},
            {   "gray",      no_argument, &opt::gray, 1},
            {"no-colorbox",  no_argument, &opt::colorbox, 0},
            {   "colorbox",  no_argument, &opt::colorbox, 1},
            {"no-timelabel", no_argument, &opt::timelabel, 0},
            {   "timelabel", no_argument, &opt::timelabel, 1},
            {"no-holder",    no_argument, &opt::holder, 0},
            {   "holder",    no_argument, &opt::holder, 1},
            {"no-spring",    no_argument, &opt::spring, 0},
            {   "spring",    no_argument, &opt::spring, 1},
            {"ttree-bottom-nodes", no_argument, &opt::ttree_bottom_nodes, 1},
            {"ttree-near-nodes",   no_argument, &opt::ttree_near_nodes,   1},
            {"ttree-find-node",    no_argument, &opt::ttree_find_node,    1},

            {"size",   required_argument, 0, 's'},
            {"res-hi", required_argument, 0, 0xff00},
            {"res-lo", required_argument, 0, 0xff01},
            {"load-field", required_argument, 0, 0xff10},

            {"ref-xy", required_argument, 0, 0xfe10},
            {"ref-S",  required_argument, 0, 0xfe11},
            {"ref-P",  required_argument, 0, 0xfe12},


            {"B",       no_argument, NULL, 'B'},
            {"V", required_argument, NULL, 'V'},
            {"S", required_argument, NULL, 'S'},
            {"G", required_argument, NULL, 'G'},
            {"P", required_argument, NULL, 'P'},
            {"Ux", required_argument, NULL, 'U'},
            {"Uy", required_argument, NULL, 'U'},

            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "nvhBVSGPU:x:y:s:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        // printf("DEBUG: '%c', ind=%d, arg=%s\n", c, option_index, optarg);
        bool fail = false;
        int  optn = 0;
        switch (c) {
        case 'v': // -v, --version
            fprintf(stderr, "vvplot %s\n", libvvhd_gitinfo);
            fprintf(stderr, "revision: %s\n", libvvhd_gitrev);
            if (libvvhd_gitdiff[0] == '\0') exit(0);
            fprintf(stderr, "git_diff: %s\n", libvvhd_gitdiff);
            exit(0);
        case 'h': // -h, --help
            (void)execlp("man", "man", "vvplot", NULL);
            fprintf(stderr, "%s: can not exec 'man': %s\n", argv[0], strerror(errno));
            exit(1);
        case 'B': // -B, --B
            opt::B = true;
            break;
        case 'V': // -V, --V SIZE
            opt::V = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%d %n", &opt::Vsize, &optn) < 1;
            fail = fail || !(opt::Vsize >= 0);
            // if (!fail) {
            //     printf("VSIZE -> %d\n", opt::Vsize);
            // }
            break;
        case 'S': // -S, --S SMIN,SMAX,SSTEP
            opt::S = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%lf , %lf , %lf %n", &opt::Smin, &opt::Smax, &opt::Sstep, &optn) < 3;
            fail = fail || !std::isfinite(opt::Smin);
            fail = fail || !std::isfinite(opt::Smax);
            fail = fail || !(opt::Smax >= opt::Smin);
            fail = fail || !std::isfinite(opt::Sstep);
            fail = fail || !(opt::Sstep>0);
            // if (!fail) {
            //     printf("SMIN -> %lf\n", opt::Smin);
            //     printf("SMAX -> %lf\n", opt::Smax);
            //     printf("SSTEP -> %lf\n", opt::Sstep);
            // }
            break;
        case 'G': // -G, --G GAMMA
            opt::G = true;
            opt::P = false;
            opt::U = '\0';
            opt::colorbox = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%lf %n", &opt::Gmax, &optn) < 1;
            fail = fail || !std::isfinite(opt::Gmax);
            fail = fail || !(opt::Gmax >= 0);
            // if (!fail) {
            //     printf("GAMMA -> %lf\n", opt::Gmax);
            // }
            break;
        case 'P': // -P, --P PMIN,PMAX
            opt::G = false;
            opt::P = true;
            opt::U = '\0';
            opt::colorbox = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%lf , %lf %n", &opt::Pmin, &opt::Pmax, &optn) < 2;
            fail = fail || !std::isfinite(opt::Pmin);
            fail = fail || !std::isfinite(opt::Pmax);
            fail = fail || !(opt::Pmax >= opt::Pmin);
            // if (!fail) {
            //     printf("PMIN -> %lf\n", opt::Pmin);
            //     printf("PMAX -> %lf\n", opt::Pmax);
            // }
            break;
        case 'U': // -U[xy], --U[xy] UMIN,UMAX
            opt::G = false;
            opt::P = false;
            opt::colorbox = true;
            if (!option_index) {
                // Short option -U
                fail = sscanf(optarg, "%c %n", &opt::U, &optn) < 1;
                fail = fail || !(opt::U=='x' || opt::U=='y');
            } else {
                // Long option Ux or Uy
                opt::U = long_options[option_index].name[1];
                fail = sscanf(optarg, "%lf , %lf %n", &opt::Umin, &opt::Umax, &optn) < 2;
                fail = fail || !std::isfinite(opt::Umin);
                fail = fail || !std::isfinite(opt::Umax);
                fail = fail || !(opt::Umax >= opt::Umin);
            }
            break;
        case 'x': // -x
            fail = sscanf(optarg, "%lf , %lf %n", &xmin, &xmax, &optn) < 2;
            fail = fail || std::isinf(xmin) || std::isinf(xmax);
            // if (!fail) {
            //     printf("XMIN -> %lf\n", xmin);
            //     printf("XMAX -> %lf\n", xmax);
            // }
            break;
        case 'y': // -y
            fail = sscanf(optarg, "%lf , %lf %n", &ymin, &ymax, &optn) < 2;
            fail = fail || std::isinf(ymin) || std::isinf(ymax);
            // if (!fail) {
            //     printf("YMIN -> %lf\n", ymin);
            //     printf("YMAX -> %lf\n", ymax);
            // }
            break;
        case 's': // --size
            fail = sscanf(optarg, "%dx%d %n", &opt::width, &opt::height, &optn) < 2;
            fail = fail || !(opt::width>=0);
            fail = fail || !(opt::height>=0);
            // if (!fail) {
            //     printf("W -> %d\n", opt::width);
            //     printf("H -> %d\n", opt::height);
            // }
            break;
        case 0xff00: // --res-hi
            fail = sscanf(optarg, "%d %n", &opt::mesh_hi.xres, &optn) < 1;
            fail = fail || !(opt::mesh_hi.xres>0);
            // if (!fail) {
            //     printf("RES_HI -> %d\n", opt::mesh_hi.xres);
            // }
            break;
        case 0xff01: // --res-lo
            fail = sscanf(optarg, "%d %n", &opt::mesh_lo.xres, &optn) < 1;
            fail = fail || !(opt::mesh_lo.xres>0);
            // if (!fail) {
            //     printf("RES_LO -> %d\n", opt::mesh_lo.xres);
            // }
            break;
        case 0xfe10: // --ref-xy
            if (strcmp(optarg, "o") == 0) opt::ref_xy = 'o';
            else if (strcmp(optarg, "f") == 0) opt::ref_xy = 'f';
            else if (strcmp(optarg, "b") == 0) opt::ref_xy = 'b';
            else if (strcmp(optarg, "bxy") == 0) opt::ref_xy = 'b';
            else if (strcmp(optarg, "bx") == 0) opt::ref_xy = 'x';
            else if (strcmp(optarg, "by") == 0) opt::ref_xy = 'y';
            else fail = true;
            optn = strlen(optarg);
            break;
        case 0xfe11: // --ref-S
            fail = sscanf(optarg, "%c %n", &opt::ref_S, &optn) < 1;
            fail = fail || !(opt::ref_S=='o' || opt::ref_S=='b' || opt::ref_S=='f');
            break;
        case 0xfe12: // --ref-P
            fail = sscanf(optarg, "%c %n", &opt::ref_P, &optn) < 1;
            fail = fail || !(opt::ref_P=='o' || opt::ref_P=='b' || opt::ref_P=='f' || opt::ref_P=='s');
            break;
        case 0xff10: // --load-field
            opt::load_field = optarg;
            optn = strlen(optarg); // supress error
            break;
        case 0:
            if (long_options[option_index].flag != 0) {
                // printf("%s -> %d\n", long_options[option_index].name, *long_options[option_index].flag);
                break;
            }
        case '?':
        default:
            exit(1);
        }

        if (optarg) {
            fail = fail || optarg[optn]!='\0';
        }

        if (fail) {
            fprintf(stderr, "%s: ", argv[0]);
            if (option_index) {
                fprintf(stderr, "option '--%s': ", long_options[option_index].name);
            } else {
                fprintf(stderr, "option '-%c': ", c);
            }
            fprintf(stderr, "bad argument '%s'\n", optarg);
            exit(1);
        }
    }


    if (argc-optind != 2) {
        if (argc-optind < 1) fprintf(stderr, "%s: missing INPUT file\n", argv[0]);
        if (argc-optind < 2) fprintf(stderr, "%s: missing TARGET path\n", argv[0]);
        if (argc-optind > 2) fprintf(stderr, "%s: too many command line arguments\n", argv[0]);
        fprintf(stderr, "Usage: vvplot [OPTIONS] INPUT TARGET. See `vvplot --help'.\n");
        exit(1);
    } else {
        opt::input = argv[optind+0];
        opt::target = argv[optind+1];

        H5Eset_auto(H5E_DEFAULT, NULL, NULL);
        if (opt::ofmt != ffmt::png || H5Fis_hdf5(opt::input) > 0) {
            opt::ifmt = ffmt::hdf;
        }

        struct stat statbuf = {};
        int err = stat(opt::target, &statbuf);
        if (!err && S_ISDIR(statbuf.st_mode)) {
            std::stringstream fout_name;

            std::string target = opt::target;
            fout_name << target;
            if (*target.crbegin() != '/') {
                fout_name << "/";
            }

            std::string input = basename(opt::input);
            size_t ext = input.find_last_of(".");
            if (ext != std::string::npos) {
                input.erase(ext, std::string::npos);
            }

            switch (opt::ofmt) {
                case ffmt::png: fout_name << input << ".png"; break;
                case ffmt::tar: fout_name << input << ".tar"; break;
            }

            opt::target = strdup(fout_name.str().c_str());
        }
    }

    /* XMIN, XMAX may be NaN, but NaNs are never swapped */
    if (xmin > xmax) { std::swap(xmin, xmax); }
    if (ymin > ymax) { std::swap(ymin, ymax); }

    if (opt::ifmt == ffmt::hdf) {
        /* Trying to adjust plot dimensions
        Exactly one parameter should be missing */
        uint8_t var_cnt = 0;
        std::string var_names = "";
        #define COMMA std::string(var_cnt?", ":"")
        if (!opt::width)  { var_names+= COMMA+ "--size (W)"; var_cnt++; }
        if (!opt::height) { var_names+= COMMA+ "--size (H)"; var_cnt++; }
        if (xmin==xmax)  { var_names+= COMMA+ "-x (XMIN, XMAX)"; var_cnt++; }
        if (ymin==ymax)  { var_names+= COMMA+ "-y (YMIN, YMAX)"; var_cnt++; }
        if (std::isnan(xmin)) { var_names+= COMMA+ "-x (XMIN)"; var_cnt++; }
        if (std::isnan(xmax)) { var_names+= COMMA+ "-x (XMAX)"; var_cnt++; }
        if (std::isnan(ymin)) { var_names+= COMMA+ "-y (YMIN)"; var_cnt++; }
        if (std::isnan(ymax)) { var_names+= COMMA+ "-y (YMAX)"; var_cnt++; }
        #undef COMMA

        if (var_cnt < 1) {
            opt::height = 0;
        } else if (var_cnt > 1) {
            fprintf(stderr, "%s: too many options ommited: %s.\n", argv[0], var_names.c_str());
            exit(1);
        }

        /* By now we have exactly one unknown dimension */
        if (!opt::width)  { opt::width  = double(opt::height)*(xmax-xmin)/(ymax-ymin); opt::width-=opt::width%2; }
        if (!opt::height) { opt::height = double(opt::width) *(ymax-ymin)/(xmax-xmin); opt::height-=opt::height%2; }
        if (xmin==xmax)  { double xc=xmin, dx=(ymax-ymin)*opt::width/opt::height/2.0; xmax=xc+dx; xmin=xc-dx; }
        if (ymin==ymax)  { double yc=ymin, dy=(xmax-xmin)*opt::height/opt::width/2.0; ymax=yc+dy; ymin=yc-dy; }
        if (std::isnan(xmin)) { xmin = xmax - (ymax-ymin)*opt::width/opt::height; }
        if (std::isnan(xmax)) { xmax = xmin + (ymax-ymin)*opt::width/opt::height; }
        if (std::isnan(ymin)) { ymin = ymax - (xmax-xmin)*opt::height/opt::width; }
        if (std::isnan(ymax)) { ymax = ymin + (xmax-xmin)*opt::height/opt::width; }

        opt::mesh_lo.dxdy = (xmax-xmin)/opt::mesh_lo.xres;
        opt::mesh_lo.yres = ceil((ymax-ymin)/opt::mesh_lo.dxdy);

        opt::mesh_hi.dxdy = (xmax-xmin)/opt::mesh_hi.xres;
        opt::mesh_hi.yres = ceil((ymax-ymin)/opt::mesh_hi.dxdy);

        opt::Vcirc = (xmax-xmin)/opt::width*opt::Vsize/2; // radius
        // fprintf(stderr, "VCIRC -> %lf\n", opt_Vcirc);
    }
}