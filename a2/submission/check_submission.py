#!/usr/bin/python3

import argparse, os, subprocess, shutil

# example program passes with 3M, but not with 2M
memory_limit_systemd = "5M"

def check_runtimes(path):
  return (   check_runtime( 1, 4, path, 0.29, 10)
         and check_runtime( 5, 5, path, 9.98, 5)
         and check_runtime(10, 5, path, 5.30, 10)
         and check_runtime(20, 5, path, 2.76, 10)
         )


def check_runtime(t, s, path, time_bd, repeat):
  bench = "bench.csv"
  warmup = 1

  time_bd_total = 2 * time_bd * (warmup + repeat)

  if os.path.exists(os.path.join(path, bench)):
    os.remove(os.path.join(path, bench))

  if os.path.exists(os.path.join(path, "cells")):
    os.remove(os.path.join(path, "cells"))
  os.symlink("/home/hpc2022/cell_distances/test_data/cell_e{}".format(s), path + "/cells")

  cell_distances_cmd = "./cell_distances -t{t}".format(t = t)
  systemd_cmd = ("systemd-run --user --scope --property=MemoryMax={memory_limit} {cmd}"
                 .format(memory_limit = memory_limit_systemd, cmd = cell_distances_cmd))
  cmd = (("hyperfine --export-csv {bench} --time-unit millisecond " +
          "--warmup {warmup} --max-runs {repeat} \"{cmd}\"")
          .format(bench = bench, warmup = warmup, repeat = repeat, cmd = systemd_cmd))

  try:
    print(cmd)
    subprocess.Popen(cmd, shell = True, cwd=path,
                     stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL).wait(time_bd_total)
  except subprocess.TimeoutExpired:
    print("TOO SLOW FOR {} ON cell_e{}".format(cell_distances_cmd, s))
    return False

  try:
    with open(os.path.join(path, bench), "r") as f:
      time = float(f.readlines()[-1].split(",")[-2])
      # print("CALIBRATE: {} ON cell_e{}-> {}".format(cell_distances_cmd, s, 2*time))
      # return True
      if time > time_bd:
        print("TOO SLOW ({} ms) FOR {} ON cell_e{}".format(time, cell_distances_cmd, s))
        return False
      else:
        return True
  except:
    pass

  print("RUNTIME ERROR FOR {} ON  cell_e{}".format(cell_distances_cmd, s))
  return False

def verify_distance_file(distreffiles, distfile):
  distrefs = [dict() for f in distreffiles]
  dist = dict()
  for (distreffile,distref) in zip(distreffiles,distrefs):
    for l in open(distreffile, 'r').readlines():
      l = l.split(" ")
      distref[l[0].strip()] = int(l[1])
  
  for l in open(distfile, 'r').readlines():
    l = l.split(" ")
    dist[l[0].strip()] = int(l[1])
  
  invtolerance = 50
  abstolerance = 3
  first_wrong = [None for _ in distrefs]
  for (dx,distref) in enumerate(distrefs):
    for k in distref.keys():
      if ( ( k not in dist and distref[k] > abstolerance ) or 
           ( k in dist and
             abs(dist[k] - distref[k]) * invtolerance > distref[k] and
             abs(dist[k] - distref[k]) > abstolerance ) ) :
        first_wrong[dx] = k
        break

  if any(k is None for k in first_wrong):
    return True

  print( "NONE OF THE REFERENCE SOLUTIONS MATCHES:" )
  for (k,distref,distreffile) in zip(first_wrong,distrefs,distreffiles):
    print( "WHEN COMPARING WITH {}".format(distreffile) )
    print( "WRONG VALUE AT {}".format(k) )
    print( "DETECTED VALUE IS {}".format(dist[k] if k in dist else "absent") )
    print( "SHOULD BE WITHIN IN 2% RELATIVE OR 3 ABSOLUTE DISTANCE OF {}".format(distref[k]) )

  return False

def run_check_build():
  # check files
  is_valid_file = lambda f: (
       f in ["makefile", "Makefile"] or
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
  subprocess.Popen(["make", "cell_distances"], cwd=extraction_path).wait()
  subprocess.Popen(["make", "clean"], cwd=extraction_path).wait()
  if not is_valid_folder(extraction_path):
    print("ADDITIONAL FILES AFTER BUILD CLEAN")
    return False

  print( "bulding..." )
  subprocess.Popen(["make", "cell_distances"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()
  return True

def run_check_correctness():
  print( "checking correctness..." )
  if os.path.exists(os.path.join(extraction_path, "cells")):
    os.remove(os.path.join(extraction_path, "cells"))
  os.symlink("/home/hpc2022/cell_distances/test_data/cell_e5",
             os.path.join(extraction_path, "cells"))

  dist_file = os.path.join("distances/", stem)

  cell_distances_cmd = "./cell_distances -t10"
  cmd = ("systemd-run --user --scope --property=MemoryMax={memory_limit} {cmd}"
         .format(memory_limit = memory_limit_systemd, cmd = cell_distances_cmd))

  with open(dist_file, 'w') as f:
    print(cmd)
    subprocess.Popen(cmd, shell = True, cwd=extraction_path, stdout = f).wait(2000)
  if os.stat(dist_file).st_size == 0:
    print("PROGRAM DOES NOT WRITE TO STDOUT")
    return False
  else:
    return verify_distance_file(["/home/hpc2022/cell_distances/test_data/dist_e5",
                                 "/home/hpc2022/cell_distances/test_data/dist_e5_alt"],
                                 dist_file)

def run_check_runtime():
  print( "checking runtimes..." )
  return check_runtimes(extraction_path)



subprocess.run(["mkdir", "-p", "extracted"])
subprocess.run(["mkdir", "-p", "distances"])

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

passed = run_check_build() and run_check_correctness() and run_check_runtime()

# clean
print( "final cleaning..." )
shutil.rmtree(extraction_path)

# feedback summary
if passed:
  print("submission passes script")
else:
  print("submission DOES NOT pass script")