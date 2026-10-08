# Action vs Service (Question 3)

I wrote the "go to charging station" task twice, as a service (version A) and as an
action (version B). The trip takes 30 s.

## Version A: service with a 5 s timeout
The client sends the request and waits 5 s. The server is still sleeping, so the
client prints `TIMED OUT` and gives up.

The timeout only happens on the client side. The server has no idea the client left,
so it keeps going and after 30 s it sends its response. Nobody is waiting for it any
more (the client removed its pending request), so it is dropped. The robot would have
driven the whole trip even though the caller already treated it as failed. Also, the
server callback sleeps for the whole 30 s, so while it runs this node can't answer
anything else on a single-threaded executor.

## Version B: action
The client sends a goal and gets accepted right away. After that it is not blocked
on one call. It receives `distance_remaining` every second (58, 56, ... 0) and then the
result, so it stays connected for the full 30 s with no timeout problem. The client can
also cancel (`action_client cancel`): the server sees `is_canceling()` in its loop and
stops. The server runs `execute()` in its own thread so the executor stays free.

I used a 60 m trip at 2 m/s so it lasts 30 s and the feedback counts down by 2 each second.

## When I would use which
- **Action**: anything that takes a while, where the caller wants progress or may need to
  cancel it. Docking, navigation (`NavigateToPose`), moving an arm along a trajectory.
- **Service**: quick request and reply, like reading or setting a parameter, asking for
  the current state, or a short trigger such as reset or enable.

If it can run for more than about a second, or someone might want to stop it, I use an action.
