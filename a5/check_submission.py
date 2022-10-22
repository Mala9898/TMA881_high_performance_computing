#!/usr/bin/python3

import argparse, os, subprocess, shutil


def check_runtimes(path):
  return (   check_runtime( 100   , 100   , "0.01" , 100000 , 1  , path , 3.93 , 10 )
         and check_runtime( 100   , 100   , "0.01" , 100000 , 10 , path , 4.23 , 10 )
         and check_runtime( 10000 , 10000 , "0.02" , 1000   , 2  , path , 93.8  , 3 )
         and check_runtime( 10000 , 10000 , "0.02" , 1000   , 5  , path , 41.7 , 3 )
         and check_runtime( 10000 , 10000 , "0.02" , 1000   , 10 , path , 24.0 , 3 )
         and check_runtime( 10000 , 1000  , "0.6"  , 200    , 1  , path , 6.93 , 3 )
         and check_runtime( 10000 , 1000  , "0.6"  , 200    , 5  , path , 4.25 , 3 )
         )

def check_runtime(width, height, d, n, p, path, time_bd, repeat):
  bench = "bench.csv"
  warmup = 1

  time_bd_total = 2 * time_bd * (warmup + repeat)

  if os.path.exists(path + "/init"):
    os.remove(path + "/init")
  os.symlink("/home/hpc2022/diffusion_opencl/test_data/init_{}_{}"
             .format(width,height), path + "/init")

  diffusion_cmd = ("/usr/lib64/openmpi/bin/mpirun -n {p} ./diffusion -d{d} -n{n}"
                   .format(p = p, d = d, n = n))
  cmd = (("hyperfine --export-csv {bench} --time-unit millisecond " +
          "--warmup {warmup} --max-runs {repeat} \"{cmd}\"")
         .format(bench = "bench.csv", warmup = warmup, repeat = repeat, cmd = diffusion_cmd))

  try:
    print(cmd)
    subprocess.Popen(cmd, shell = True, cwd=path,
                     stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL).wait(time_bd_total)
  except subprocess.TimeoutExpired:
    print("TOO SLOW FOR {} ON init_{}_{}".format(diffusion_cmd, width, height))
    return False

  try:
    with open(os.path.join(path, bench), "r") as f:
      time = float(f.readlines()[-1].split(",")[-2])
      # print("CALIBRATE: {} ON init_{}_{} -> {}".format(diffusion_cmd, width, height, 2*time))
      # return True
      if time > time_bd:
        print("TOO SLOW ({} ms) FOR {} ON init_{}_{}".format(time, diffusion_cmd, width, height))
        return False
      else:
        return True
  except:
    pass

  print("RUNTIME ERROR FOR {} ON init_{}_{}".format(diffusion_cmd, width, height))
  return False

def run_check_build():
  # check files
  is_valid_file = lambda f: (
       f in ["makefile", "Makefile"] or
       f.endswith(".incl") or f.endswith(".cl") or
       f.endswith(".cc") or f.endswith(".c") or
       f.endswith(".hh") or f.endswith(".h") )
  is_valid_folder = lambda folder: (
    all( all(map(is_valid_file, files))
         for (root, _, files) in os.walk(folder) ) )

  print( "checking for additional files..." )
  if not is_valid_folder(extraction_path):
    print("ADDITIONAL FILES IN TAR")
    return False

  # build clean build
  print( "bulding and cleaning..." )
  subprocess.Popen(["make", "diffusion"], cwd=extraction_path).wait()
  subprocess.Popen(["make", "clean"], cwd=extraction_path).wait()
  if not is_valid_folder(extraction_path):
    print("ADDITIONAL FILES AFTER BUILD CLEAN")
    return False

  print( "bulding..." )
  subprocess.Popen(["make", "diffusion"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()
  return True

def run_check_runtime():
  print( "checking runtimes..." )
  return check_runtimes(extraction_path)



subprocess.run(["mkdir", "-p", "extracted"])

parser = argparse.ArgumentParser()
parser.add_argument("tarfile")
args = parser.parse_args()

tar_file = args.tarfile
assert( tar_file.endswith(".tar.gz") )

stem = tar_file[:-7]
while stem.find("/") != -1:
  stem = stem[stem.find("/")+1:]

extraction_path = "extracted/" + stem
if os.path.isdir(extraction_path):
  print("FATAL: extreaction path {} exists already".format(extraction_path))
  exit(1)

print( "extracting..." )
subprocess.run(["mkdir", extraction_path])
subprocess.run(["tar", "xf", tar_file, "-C", extraction_path])

passed = run_check_build() and run_check_runtime()

# clean
print( "final cleaning..." )
shutil.rmtree(extraction_path)

# feedback summary
if passed:
  print("submission passes script")
  print("check results manually")
else:
  print("submission DOES NOT pass script")