function heat_diffusion(
    width::Int,
    height::Int,
    initial_value::F,
    diffusion_constant::F,
    nmbiterations::Int
   ) where {F <: AbstractFloat}
  box = fill(zero(F), height, width)
  box[min(1,div(height,2)),min(1,div(width,2))] = initial_value

  function heat(box, ix, jx)
    n = ix > 1      ? box[ix-1,jx] : zero(F)
    s = ix < height ? box[ix+1,jx] : zero(F)
    w = jx > 1      ? box[ix,jx-1] : zero(F)
    e = jx < width  ? box[ix,jx+1] : zero(F)
    return box[ix,jx] + diffusion_constant * ( (n + s + w + e)/4 - box[ix,jx] )
  end

  for _ in 1:nmbiterations
    box = [heat(box,ix,jx) for ix in 1:height, jx in 1:width]
  end

  average = sum(box) / (width*height)
  println("Average: $average")

  average_absdiff = sum(abs.(box .- average)) / (width*height)
  println("Average absolute difference: $average_absdiff")
end
