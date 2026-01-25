get_target_build_options() {
    local target_name=$1

    case $target_name in
        x9lite)
            BUILD_OPTIONS+="-DPCB=X9LITE"
            ;;
        x9lites)
            BUILD_OPTIONS+="-DPCB=X9LITES"
            ;;
        x7)
            BUILD_OPTIONS+="-DPCB=X7"
            ;;
        x7access)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=ACCESS -DPXX1=YES"
            ;;
        t12)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T12 -DINTERNAL_MODULE_MULTI=ON"
            ;;
        tx12)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TX12"
            ;;
        tx12mk2)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TX12MK2"
            ;;
        gx12)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=GX12"
            ;;
        boxer)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=BOXER"
            ;;
        t8)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T8"
            ;;
        zorro)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=ZORRO"
            ;;
        pocket)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=POCKET"
            ;;
        mt12)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=MT12"
            ;;
        tlite)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TLITE"
            ;;
        tlitef4)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TLITEF4"
            ;;
        tpro)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TPRO"
            ;;
        tprov2)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TPROV2"
            ;;
        tpros)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TPROS"
            ;;
        bumblebee)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=BUMBLEBEE"
            ;;
        t20)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T20"
            ;;
        t12max)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T12MAX"
            ;;
        t14)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T14"
            ;;
        t20v2)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T20V2"
            ;;
        lr3pro)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=LR3PRO"
            ;;
        commando8)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=COMMANDO8"
            ;;
        xlite)
            BUILD_OPTIONS+="-DPCB=XLITE"
            ;;
        xlites)
            BUILD_OPTIONS+="-DPCB=XLITES"
            ;;
        x9d)
            BUILD_OPTIONS+="-DPCB=X9D"
            ;;
        x9dp)
            BUILD_OPTIONS+="-DPCB=X9D+"
            ;;
        x9dp2019)
            BUILD_OPTIONS+="-DPCB=X9D+ -DPCBREV=2019"
            ;;
        x9e)
            BUILD_OPTIONS+="-DPCB=X9E"
            ;;
        x9e-hall)
            BUILD_OPTIONS+="-DPCB=X9E -DSTICKS=HORUS"
            ;;
        x10)
            BUILD_OPTIONS+="-DPCB=X10"
            ;;
        x10express)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=EXPRESS -DPXX1=YES"
            ;;
        x12s)
            BUILD_OPTIONS+="-DPCB=X12S"
            ;;
        t15)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=T15 -DINTERNAL_MODULE_CRSF=ON"
            ;;
        t16)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=T16 -DINTERNAL_MODULE_MULTI=ON"
            ;;
        t18)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=T18"
            ;;
        t15pro)
            BUILD_OPTIONS+="-DPCB=T15PRO"
            ;;
        tx15)
            BUILD_OPTIONS+="-DPCB=TX15"
            ;;
        tx16s)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=TX16S"
            ;;
        tx16smk3)
            BUILD_OPTIONS+="-DPCB=TX16SMK3"
            ;;
        f16)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=F16"
            ;;
        v12)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=V12"
            ;;
        v14)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=V14"
            ;;
        v16)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=V16"
            ;;
        nv14)
            BUILD_OPTIONS+="-DPCB=PL18 -DPCBREV=NV14"
            ;;
        el18)
            BUILD_OPTIONS+="-DPCB=PL18 -DPCBREV=EL18"
            ;;
        pl18)
            BUILD_OPTIONS+="-DPCB=PL18"
            ;;
        pl18ev)
            BUILD_OPTIONS+="-DPCB=PL18 -DPCBREV=PL18EV"
            ;;
        pl18u)
            BUILD_OPTIONS+="-DPCB=PL18 -DPCBREV=PL18U"
            ;;
        nb4p)
            BUILD_OPTIONS+="-DPCB=PL18 -DPCBREV=NB4P"
            ;;
        st16)
            BUILD_OPTIONS+="-DPCB=ST16"
            ;;
        pa01)
            BUILD_OPTIONS+="-DPCB=PA01"
            ;;
        *)
            echo "Unknown target: $target_name"
            return 1
            ;;
    esac
}

# Determine parallel job limit based on environment
determine_max_jobs() {
  if [[ -n ${CMAKE_BUILD_PARALLEL_LEVEL} ]]; then
    MAX_JOBS=${CMAKE_BUILD_PARALLEL_LEVEL}
  elif [ -n "$GITHUB_ACTIONS" ]; then
    # Limit jobs in GitHub Actions to n-1 to avoid resource contention
    if [ "$(uname)" = "Darwin" ]; then
      MAX_JOBS=2  # macOS runners have 3 cores
    else
      MAX_JOBS=3  # Linux and Windows runners have 4 cores
    fi
  else
    MAX_JOBS=""  # Let CMake/build system decide
  fi
  export MAX_JOBS
}

# Helper function to run cmake build with appropriate parallelism
cmake_build_parallel() {
  local args=()
  local native_flags=""
  
  # Separate cmake args from native flags (anything after --)
  for arg in "$@"; do
    if [[ "$arg" == "--" ]]; then
      # Capture everything after -- as native flags
      shift
      native_flags="-- $*"
      break
    fi
    args+=("$arg")
    shift
  done
  
  if [[ -n ${MAX_JOBS} ]]; then
    cmake --build "${args[@]}" --parallel ${MAX_JOBS} ${native_flags}
  else
    cmake --build "${args[@]}" --parallel ${native_flags}
  fi
}
