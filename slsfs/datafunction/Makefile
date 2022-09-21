

function:
	CPPBUILDARGS="-DAS_ACTIONLOOP -O3 -DNDEBUG" ./push.sh && ./deploy.sh

exec: *.cpp *.hpp
	CPPBUILDARGS="-ggdb" ./push.sh && chmod +x exec

deploy:
	wsk -i action update hare1039/slsfs-actionproxy --docker 'openwhisk/actionloop-base:nightly'

main.zip:
	docker run -i --rm 'action-golang-v1.16:nightly' -compile main <$(SRC) >$(ZIP)
