request = function()
  local e = math.random(1, 100)

  if e < 86 then
    wrk.method = "GET"

    if e < 21 then
      path = "/users/" .. math.random(1, 10000 )
    elseif e < 41 then
      path = "/locations/" .. math.random(1, 100000 )
    elseif e < 51 then
      path = "/visits/" .. math.random(1, 10000 )
    elseif e < 61 then
      path = "/users/" .. math.random(1, 10000 ) .. "/visits"
    else
      path = "/locations/" .. math.random(1, 10000 ) .. "/avg"
    end

  else
    wrk.method = "POST"
    wrk.body = "{}"
    wrk.headers["Content-Type"] = "application/json"

    if e < 89 then
      path = "/users/" .. math.random(1, 10000 )
    elseif e < 93 then
      path = "/locations/" .. math.random(1, 100000 )
    elseif e < 95 then
      path = "/visits/" .. math.random(1, 100000 )
    elseif e < 96 then
      path = "/users/new"
    elseif e < 98 then
      path = "/visits/new"
    else
      path = "/locations/new"
    end
  end

  return wrk.format(nil, path)
end

