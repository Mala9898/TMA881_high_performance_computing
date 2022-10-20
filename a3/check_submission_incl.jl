include("$hpc_folder/common/check_submission.jl")


struct CheckCaseNewtonIteration <: CheckCase
  num_lines    :: Int
  degree       :: Int
  num_threads  :: Int
  picture_path :: Union{Nothing,String}
  hyperfine    :: HyperfineSetup
end

function CheckCaseNewtonIteration(
    num_lines    :: Int,
    degree       :: Int,
    hyperfine    :: HyperfineSetup
  )
  @assert hascore_limit(hyperfine)
  CheckCaseNewtonIteration(
    num_lines,
    degree,
    core_limit(hyperfine),
    nothing,
    hyperfine
    )
end

function CheckCaseNewtonIteration(
    num_lines    :: Int,
    degree       :: Int,
    picture_path :: String,
    hyperfine    :: HyperfineSetup
  )
  if !ispath(picture_path)
    mkpath(picture_path)
  end
  @assert hascore_limit(hyperfine)
  CheckCaseNewtonIteration(
    num_lines,
    degree,
    core_limit(hyperfine),
    picture_path,
    hyperfine
    )
end


function Base.show(io::IO, case::CheckCaseNewtonIteration)
  show(io, "$(case.num_lines) lines for degree $(case.degree) on $(case.num_threads) threads")
end

hyperfine_setup(case::CheckCaseNewtonIteration) = case.hyperfine

build_exec_name(::Type{CheckCaseNewtonIteration}) = "newton"

function command(case::CheckCaseNewtonIteration) :: Cmd
  return command(hyperfine_setup(case),
    `./$(build_exec_name(CheckCaseNewtonIteration))
      -l$(case.num_lines) -t$(case.num_threads) $(case.degree)`)
end

function verify(
    case::CheckCaseNewtonIteration,
    stdout_log::String
  ) :: Union{Nothing,ErrorException}
  afileroot = "newton_attractors_x$(case.degree)"
  cfileroot = "newton_convergence_x$(case.degree)"
  
  open("$afileroot.ppm") do f
    if startswith(readline(f), "P3")
      return ErrorException(
        "Error when checking header of $afileroot.ppm: Must be P3 formate.")
    end
  end
  open("$cfileroot.ppm") do f
    if startswith(readline(f), "P3")
      return ErrorException(
        "Error when checking header of $cfileroot.ppm: Must be P3 formate.")
    end
  end

  if !isnothing(case.picture_path)
    println("Converting both pictures to png and copying for manual check.")
    afilepathpng = joinpath(case.picture_path, "$afileroot.png")
    cfilepathpng = joinpath(case.picture_path, "$cfileroot.png")
    run(`convert $afileroot.ppm $afilepathpng`)
    run(`convert $cfileroot.ppm $cfilepathpng`)
  end

  rm("$afileroot.ppm")
  rm("$cfileroot.ppm")
end
