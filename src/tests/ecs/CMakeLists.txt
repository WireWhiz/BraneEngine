# src/tests/ecs
add_test("common.ecs.EntityManager.testing_test" ${TestDir} 1)
set_tests_properties("common.ecs.EntityManager.testing_test" PROPERTIES WILL_FAIL TRUE)