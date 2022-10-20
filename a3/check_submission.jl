#!/usr/bin/julia

const hpc_folder = dirname(dirname(@__FILE__))

include("check_submission_incl.jl")

pics = joinpath(pwd(), "pictures",
                basename(first(ARGS))[1:end-7])

check(ARGS, CheckCaseNewtonIteration[
  CheckCaseNewtonIteration(1000, 1,       HyperfineSetup( 1, 0.10, 10, 5)),
  CheckCaseNewtonIteration(1000, 2, pics, HyperfineSetup( 1, 0.21, 10, 2)),
  CheckCaseNewtonIteration(1000, 5,       HyperfineSetup( 1, 0.64, 10, 2)),
  CheckCaseNewtonIteration(1000, 7,       HyperfineSetup( 1, 1.20,  5, 1)),

  CheckCaseNewtonIteration(1000, 5,       HyperfineSetup( 1, 0.64, 10, 2)),
  CheckCaseNewtonIteration(1000, 5,       HyperfineSetup( 2, 0.34, 10, 2)),
  CheckCaseNewtonIteration(1000, 5,       HyperfineSetup( 3, 0.24, 10, 2)),
  CheckCaseNewtonIteration(1000, 5,       HyperfineSetup( 4, 0.17, 10, 5)),

  CheckCaseNewtonIteration(1000, 7,       HyperfineSetup(10, 0.18, 10, 2)),
  CheckCaseNewtonIteration(2000, 7,       HyperfineSetup(10, 0.56, 10, 2)),
  CheckCaseNewtonIteration(3000, 7, pics, HyperfineSetup(10, 1.26,  5, 1)),
  ])
println("Submission passes the check script. Verify pictures in $pics manually.")
